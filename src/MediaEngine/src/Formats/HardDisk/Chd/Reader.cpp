#include "Container.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/ReadRejection.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ChdHardDisk {
const ReaderCapabilities& Reader::capabilities()const{
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{readLayout(context,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;auto layout=readLayout(context,token);const auto& path=context.source()->primaryPath();auto source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));std::vector<DataRange> ranges;
        for(std::uint64_t i=0;i<layout.hunkCount;++i){checkCancellation(token);const auto stored=qFromBigEndian<std::uint32_t>(layout.map.data()+std::size_t(i)*UncompressedMapBytes);const auto address=std::int64_t(i*layout.hunkBytes),length=std::min(std::int64_t(layout.hunkBytes),layout.logicalBytes-address);
            if(!stored){addBlockRange(ranges,address,length,DataRangeKind::Zero);continue;}
            const auto offset=std::uint64_t(stored)*layout.hunkBytes;if(offset>std::uint64_t(context.length())||layout.hunkBytes>std::uint64_t(context.length())-offset)throw MediaError(DiagnosticCode::ChdHardDiskHunk,{std::to_string(i)});
            addBlockRange(ranges,address,length,DataRangeKind::Stored,source,std::int64_t(offset));}
        auto blocks=std::make_shared<BlockImage>(layout.logicalBytes,std::int32_t(layout.unitBytes),std::move(ranges),std::move(layout.geometry));
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VersionKey),std::to_string(Version5)},{std::string(VariantKey),std::string(FixedVariant)},{std::string(HunkSizeKey),std::to_string(layout.hunkBytes)}});
    });
}
}
