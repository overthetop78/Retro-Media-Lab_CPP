#include "Structures.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhdx {
namespace {
std::vector<DataRange> readRanges(const RecognitionContext& context,std::shared_ptr<const MediaFileSystems::RandomAccessData> source,const Structures& structures,std::stop_token token){
    using namespace Constants;const auto& meta=structures.metadata;const auto count=std::uint64_t(meta.virtualSize)/meta.blockSize+(meta.virtualSize%meta.blockSize!=0),ratio=BitmapSectors*meta.logicalSector/meta.blockSize;if(!ratio)throw MediaError(DiagnosticCode::VhdxChunkRatio);const auto entries=count+(count-1)/ratio,bytes=entries*EntryBytes;if(bytes>structures.bat.length||bytes>MaximumBuffer)throw MediaError(DiagnosticCode::VhdxBatSize);const auto bat=context.readAsync(structures.bat.offset,std::int32_t(bytes),token).get();std::vector<DataRange> ranges;
    for(std::uint64_t payload=0;payload<count;++payload){checkCancellation(token);const auto index=payload+payload/ratio,entry=qFromLittleEndian<std::uint64_t>(bat.data()+std::size_t(index)*EntryBytes),state=entry&StateMask;const auto address=std::int64_t(payload*meta.blockSize),length=std::min(std::int64_t(meta.blockSize),meta.virtualSize-address);
        switch(state){
        case Zero:addBlockRange(ranges,address,length,DataRangeKind::Zero);break;
        case NotPresent:case Undefined:case Unmapped:addBlockRange(ranges,address,length,DataRangeKind::Unallocated);break;
        case FullyPresent:{const auto offset=entry&FileOffsetMask;if(offset>std::uint64_t(context.length())||std::uint64_t(length)>std::uint64_t(context.length())-offset)throw MediaError(DiagnosticCode::VhdxBatRange,{std::to_string(index)});addBlockRange(ranges,address,length,DataRangeKind::Stored,source,std::int64_t(offset));break;}
        case PartiallyPresent:throw MediaError(DiagnosticCode::VhdxPartial);
        default:throw MediaError(DiagnosticCode::VhdxBatState,{std::to_string(index),std::to_string(state)});
        }
    }return ranges;
}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{std::vector<std::uint8_t>(FileSignature.begin(),FileSignature.end())},{},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{readStructures(context,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto structures=readStructures(context,token);const auto& path=context.source()->primaryPath();auto source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));if(source->length()!=std::uint64_t(context.length()))throw MediaError(DiagnosticCode::VhdxChanged);auto ranges=readRanges(context,source,structures,token);const auto& meta=structures.metadata;auto blocks=std::make_shared<const BlockImage>(meta.virtualSize,meta.logicalSector,std::move(ranges));return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VariantKey),std::string((meta.fileFlags&LeaveAllocated)?FixedVariant:DynamicVariant)},{std::string(Page83Key),guidText(meta.page83)},{std::string(PhysicalSectorKey),std::to_string(meta.physicalSector)}});});}
}
