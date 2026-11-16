#include "RawImages.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/AtomicImageWriter.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>

namespace RetroMediaLab::MediaEngine::Definitions {
namespace {
namespace M=MediaFileSystems;
struct Plan {
    DiskDefinition definition;
    std::vector<Sector> sectors;
    std::uint64_t length=0;
};
using Plans=std::map<std::string,Plan,IdentifierLess>;
std::shared_ptr<const Plans> prepare(Catalogue definitions){
    auto result=std::make_shared<Plans>();
    for(auto& [id,definition]:definitions){
        auto layout=definition.layout();std::uint64_t length=0;
        for(const auto& sector:layout){
            if(std::uint64_t(sector.imageSize)>std::uint64_t(std::numeric_limits<std::int64_t>::max())-length)throw MediaError(DiagnosticCode::DefinitionValue,{std::string(Constants::CapacityField),id});
            length+=std::uint64_t(sector.imageSize);
        }
        result->emplace(id,Plan{std::move(definition),std::move(layout),length});
    }
    return result;
}
const Plan& require(const Plans& plans,std::string_view id){const auto found=plans.find(id);if(found==plans.end())throw MediaError(DiagnosticCode::DefinedRawFormat,{std::string(id)});return found->second;}
bool geometryMatches(const M::SectorImage& image,const Plan& plan){const auto& geometry=image.geometry();return std::int64_t(geometry.cylinders)==plan.definition.cylinders&&std::int64_t(geometry.heads)==plan.definition.heads&&image.blockCount()==plan.sectors.size();}
bool canWrite(const M::SectorImage& image,const Plan& plan){
    if(!geometryMatches(image,plan))return false;
    for(const auto& sector:plan.sectors){const auto block=image.block(sector.logicalBlock);if(!block||block->data.size()!=std::size_t(sector.size))return false;}
    return true;
}
[[noreturn]] void invalidSector(const Plan& plan,const Sector& sector){throw MediaError(DiagnosticCode::DefinedRawSector,{std::to_string(sector.cylinder),std::to_string(sector.head),std::to_string(sector.number),plan.definition.id});}
std::shared_ptr<const M::SectorImage> read(const Plan& plan,M::Bytes bytes,std::stop_token token){
    if(bytes.size()!=plan.length)throw MediaError(DiagnosticCode::DefinedRawLength,{plan.definition.id,std::to_string(bytes.size()),std::to_string(plan.length)});
    if(plan.sectors.empty())throw MediaError(DiagnosticCode::DefinitionValue,{std::string(Constants::Sectors),plan.definition.id});
    std::vector<M::SectorBlock> blocks;blocks.reserve(plan.sectors.size());
    std::size_t offset=0;
    for(const auto& sector:plan.sectors){
        checkCancellation(token);
        if(offset>bytes.size()||std::size_t(sector.size)>bytes.size()-offset)invalidSector(plan,sector);
        M::SectorBlock block;block.logicalBlock=sector.logicalBlock;block.address={sector.cylinder,sector.head,sector.number};
        block.data.assign(bytes.begin()+std::ptrdiff_t(offset),bytes.begin()+std::ptrdiff_t(offset+sector.size));block.integrityValid=std::nullopt;
        blocks.push_back(std::move(block));offset+=std::size_t(sector.imageSize);
    }
    return createImage(plan.definition,plan.sectors,std::move(blocks),token);
}
void write(const M::SectorImage& image,const Plan& plan,const std::filesystem::path& path,std::stop_token token){
    checkCancellation(token);
    if(!geometryMatches(image,plan))throw MediaError(DiagnosticCode::DefinedRawGeometry,{plan.definition.id});
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){
        for(const auto& sector:plan.sectors){
            checkCancellation(stop);const auto block=image.block(sector.logicalBlock,stop);
            if(!block||block->data.size()!=std::size_t(sector.size))invalidSector(plan,sector);
            output(block->data);
            if(sector.imageSize>sector.size){const std::vector<std::uint8_t> padding(std::size_t(sector.imageSize-sector.size));output(padding);}
        }
    },token);
}
class Reader final:public ImageReader {
    std::shared_ptr<const Plans> plans_;
    ReaderCapabilities capabilities_;
public:
    explicit Reader(std::shared_ptr<const Plans> plans):plans_(std::move(plans)){
        for(const auto& [id,plan]:*plans_)capabilities_.formatIds.push_back(id);
        capabilities_.extensions={std::string(Constants::RawExtension)};capabilities_.mediaKinds={MediaKind::Floppy};capabilities_.representationKinds={RepresentationKind::Sectors};
    }
    const ReaderCapabilities& capabilities()const override{return capabilities_;}
    bool supportsFormatId(std::string_view id)const override{return plans_->contains(id);}
    std::future<bool> canReadAsync(RecognitionContext context,std::stop_token token)const override{
        return std::async(std::launch::deferred,[plans=plans_,context,token]{checkCancellation(token);const auto id=context.requestedFormatId();if(!id)return false;const auto found=plans->find(*id);return found!=plans->end()&&context.length()>=0&&std::uint64_t(context.length())==found->second.length;});
    }
    std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext context,std::stop_token token)const override{
        return std::async(std::launch::deferred,[plans=plans_,context,token]{checkCancellation(token);const auto id=context.requestedFormatId();const auto& plan=require(*plans,id.value_or(std::string{}));const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(plan,bytes,token));});
    }
};
}
std::shared_ptr<const MediaFileSystems::SectorImage> createImage(const DiskDefinition& definition,std::span<const Sector> layout,std::vector<MediaFileSystems::SectorBlock> blocks,std::stop_token token){
    checkCancellation(token);if(layout.empty())throw MediaError(DiagnosticCode::DefinitionValue,{std::string(Constants::Sectors),definition.id});
    std::uint32_t maximumSize=0,maximumCount=0;std::uint64_t capacity=0;
    std::map<std::pair<std::int32_t,std::int32_t>,std::uint32_t> counts;
    for(const auto& sector:layout){
        checkCancellation(token);
        if(sector.size<=0||sector.imageSize<=0||std::uint64_t(sector.imageSize)>std::uint64_t(std::numeric_limits<std::int64_t>::max())-capacity)throw MediaError(DiagnosticCode::DefinitionValue,{std::string(Constants::CapacityField),definition.id});
        maximumSize=std::max(maximumSize,std::uint32_t(sector.size));maximumCount=std::max(maximumCount,++counts[{sector.cylinder,sector.head}]);capacity+=std::uint64_t(sector.imageSize);
    }
    MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=true;options.capacity=capacity;options.logicalBlockCount=layout.size();
    return sparseSectorImage({definition.id,maximumSize,std::uint32_t(definition.cylinders),std::uint32_t(definition.heads),maximumCount},std::move(blocks),options);
}
std::shared_ptr<const ImageReader> rawReader(Catalogue catalogue){return std::make_shared<Reader>(prepare(std::move(catalogue)));}
std::shared_ptr<const ImageReader> rawReader(){return rawReader(rawDefinitions());}
std::shared_ptr<const ImageWriter> rawWriter(){return rawWriter(rawDefinitions());}
std::shared_ptr<const ImageWriter> rawWriter(Catalogue catalogue){
    const auto plans=prepare(std::move(catalogue));std::vector<std::string> ids;for(const auto& [id,plan]:*plans)ids.push_back(id);
    return sectorWriterAdapter(std::string(Constants::RawWriterId),std::move(ids),{std::string(Constants::RawExtension)},
        [plans](const auto& image,const auto&,auto target,auto){const auto found=plans->find(target);return found!=plans->end()&&Primitives::ordinalIdentifierEqualsIgnoreCase(image.geometry().formatId,target)&&canWrite(image,found->second);},
        [plans](const auto& image,const auto&,const auto& path,auto target,auto token){write(image,require(*plans,target),path,token);return std::vector<std::filesystem::path>{path};});
}
}
