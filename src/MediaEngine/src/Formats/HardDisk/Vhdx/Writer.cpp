#include "Writing.h"
#include "../../../Sources/AtomicImageWriter.h"
#include "../Variant.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhdx {
namespace {
std::uint32_t selectBlockSize(std::int64_t capacity){using namespace Constants;auto size=DefaultBlockSize;while(std::uint64_t(capacity)>std::uint64_t(size)*MaximumField&&size<MaximumBlockSize)size*=BlockSizeGrowth;return size;}
void writeContainer(const BlockImage& blocks,bool fixed,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto size=selectBlockSize(blocks.capacity());const auto capacity=std::uint64_t(blocks.capacity()),count=capacity/size+(capacity%size!=0),ratio=BitmapSectors*blocks.logicalBlockSize()/size,entries=count+(count-1)/ratio,batBytes=entries*EntryBytes,batLength=(batBytes+MiB-1)/MiB*MiB;
    if(batLength>MaximumField||batBytes>MaximumBuffer)throw MediaError(DiagnosticCode::VhdxWriteBat);const auto logOffset=BatRegionOffset+batLength,payloadOffset=logOffset+MiB;if(count>(MaximumSigned-payloadOffset)/size)throw MediaError(DiagnosticCode::VhdxWriteOffset);
    std::uint64_t writtenLength=0;
    const auto writeAt=[&](std::uint64_t offset,std::span<const std::uint8_t> data){checkCancellation(token);output.seek(offset);output.write(data);writtenLength=std::max(writtenLength,offset+data.size());};
    writeAt(0,createIdentifier());const auto fileId=randomGuid(),dataId=randomGuid();writeAt(Header1Offset,createHeader(FirstSequence,fileId,dataId,logOffset));writeAt(Header2Offset,createHeader(SecondSequence,fileId,dataId,logOffset));const auto regions=createRegions(std::uint32_t(batLength));writeAt(Region1Offset,regions);writeAt(Region2Offset,regions);writeAt(MetadataRegionOffset,createMetadata(blocks,size,fixed));
    std::vector<std::uint8_t> bat(static_cast<std::size_t>(batBytes)),data(size);
    for(std::uint64_t index=0;index<count;++index){checkCancellation(token);std::fill(data.begin(),data.end(),0);const auto address=std::int64_t(index*size),length=std::min(std::int64_t(size),blocks.capacity()-address);blocks.readExactly(address,std::span(data).first(std::size_t(length)),token);const auto batIndex=index+index/ratio;auto entry=Zero;
        if(fixed||!std::all_of(data.begin(),data.begin()+std::size_t(length),[](auto value){return value==0;})){const auto offset=payloadOffset+index*size;entry=offset|FullyPresent;writeAt(offset,data);}qToLittleEndian(entry,bat.data()+std::size_t(batIndex)*EntryBytes);
    }
    writeAt(BatRegionOffset,bat);const auto minimum=payloadOffset+(fixed?count*size:0);if(writtenLength<minimum){const std::array<std::uint8_t,1> zero{};writeAt(minimum-1,zero);}
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&(blocks->logicalBlockSize()==Constants::LegacySector||blocks->logicalBlockSize()==Constants::LargeSector)&&blocks->hasCompleteReadableCoverage();}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token]{checkCancellation(token);const auto extension=output.extension().u8string();if(!document||!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::VhdxWrite);const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());const auto fixed=HardDiskContainers::selectFixedVariant(*document,blocks,Constants::FormatId);writeAtomicSeekableImage(output,[&](const auto& stream,auto stop){writeContainer(blocks,fixed,stream,stop);},token);return std::vector<std::filesystem::path>{output};});}
}
