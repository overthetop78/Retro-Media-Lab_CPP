#include "DiskState.h"
#include "../Models/Sectors/SectorImages.h"
#include "../Sources/RandomAccessSources.h"
#include <set>
#include <QString>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
std::shared_ptr<const MediaDocument> RuntimeDisk::State::updated()const{
    std::shared_ptr<const M::ImageRepresentation> representation;
    if(const auto* image=sectors()){
        std::map<std::uint64_t,M::SectorBlock> values;
        image->visitAvailableBlocks([&](auto index){const auto cached=originals.find(static_cast<std::int64_t>(index));const auto block=cached!=originals.end()?cached->second:image->block(index);if(block)values.emplace(index,*block);return true;});
        for(const auto& [index,data]:changes){
            const auto number=static_cast<std::uint64_t>(index);auto found=values.find(number);
            if(found==values.end()){M::SectorBlock block;block.logicalBlock=number;block.address=address(index);block.data=data;found=values.emplace(number,std::move(block)).first;}
            else {auto& block=found->second;block.data=data;block.integrityValid=block.headerIntegrityValid!=false;block.dataIntegrityValid=true;block.diagnosticCode.reset();block.recordedErrorCheckCode.clear();block.recordedDataPrefix.clear();}
            if(const auto mark=dataMarks.find(index);mark!=dataMarks.end())found->second.deletedDataMark=mark->second;
        }
        std::vector<M::SectorBlock> blocks;blocks.reserve(values.size());for(auto& [index,block]:values)blocks.push_back(std::move(block));
        M::SectorImageOptions options;options.allowVariableBlockSize=image->allowsVariableBlockSize();options.capacity=image->capacity();options.logicalBlockCount=image->blockCount();
        representation=std::make_shared<SectorRepresentation>(sparseSectorImage(image->geometry(),std::move(blocks),options));
    }else{
        const auto& blockImage=*blocks();std::set<std::int64_t> boundaries{0,blockImage.capacity()};
        for(const auto& range:blockImage.ranges()){boundaries.insert(range.address());boundaries.insert(range.address()+range.length());}
        for(const auto& [index,data]:changes){const auto start=index*size();if(start>blockImage.capacity()-size())throw MediaError(DiagnosticCode::RuntimeDiskSector);boundaries.insert(start);boundaries.insert(start+size());}
        std::vector<DataRange> ranges;const std::vector<std::int64_t> points(boundaries.begin(),boundaries.end());std::size_t sourceIndex=0;
        for(std::size_t i=0;i+1<points.size();++i){
            const auto start=points[i],length=points[i+1]-start;
            if(const auto changed=changes.find(start/size());changed!=changes.end())ranges.emplace_back(start,length,DataRangeKind::Stored,memorySource(changed->second),start%size());
            else{
                while(sourceIndex<blockImage.ranges().size()&&blockImage.ranges()[sourceIndex].address()+blockImage.ranges()[sourceIndex].length()<=start)++sourceIndex;
                if(sourceIndex==blockImage.ranges().size()||blockImage.ranges()[sourceIndex].address()>start)ranges.emplace_back(start,length,DataRangeKind::Unavailable);
                else {const auto& range=blockImage.ranges()[sourceIndex];ranges.emplace_back(start,length,range.kind(),range.source(),range.kind()==DataRangeKind::Stored?range.sourceOffset()+start-range.address():0);}
            }
        }
        representation=std::make_shared<BlockImage>(blockImage.capacity(),blockImage.logicalBlockSize(),std::move(ranges),blockImage.geometry());
    }
    return std::make_shared<MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::move(representation),
        std::vector<M::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),std::vector<M::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
}
void RuntimeDisk::State::flush(std::stop_token token){
    alive();if(!dirty())return;checkCancellation(token);auto value=updated();auto source=value->mediaSource()->options();source.drive.reset();
    value=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(source)),value->formatId(),value->mediaKind(),value->representationOwner(),
        std::vector<M::VolumeDescriptor>(value->volumes().begin(),value->volumes().end()),std::vector<M::MessageRecipe>(value->messages().begin(),value->messages().end()),value->metadata(),std::vector<std::string>(value->diagnostics().begin(),value->diagnostics().end()));
    const auto path=QString::fromUtf8(document->mediaSource()->primaryPath()).toStdU16String();
    services->writing.writingService().writeAsync(value,std::filesystem::path(path),document->formatId(),token).get();
    auto options=document->mediaSource()->options();options.knownLength.reset();options.requestedFormatId=document->formatId();
    auto reopened=services->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
    document=std::move(reopened);changes.clear();dataMarks.clear();originals.clear();documentChanged=false;
}
void RuntimeDisk::State::close(){
    if(closed)return;std::exception_ptr failure;
    try{flush({});}catch(...){failure=std::current_exception();}
    try{if(ownsMechanics)document->mediaSource()->options().drive->reset();}catch(...){if(!failure)failure=std::current_exception();}
    changes.clear();dataMarks.clear();originals.clear();documentChanged=false;closed=true;
    if(failure)std::rethrow_exception(failure);
}
}
