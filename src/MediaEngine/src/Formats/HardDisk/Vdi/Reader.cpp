#include "Container.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <unordered_set>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vdi {
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
        try{readHeader(context,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto header=readHeader(context,token);
        const auto& path=context.source()->primaryPath();auto source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));
        const auto map=context.readAsync(header.mapOffset,std::int32_t(std::uint64_t(header.blocksCount)*EntryBytes),token).get();std::vector<DataRange> ranges;std::unordered_set<std::uint32_t> used;
        for(std::uint32_t logical=0;logical<header.blocksCount;++logical){checkCancellation(token);
            const auto entry=qFromLittleEndian<std::uint32_t>(map.data()+std::size_t(logical)*EntryBytes);const auto address=std::int64_t(logical)*header.blockSize;const auto length=std::min(std::int64_t(header.blockSize),header.diskSize-address);
            if(entry==Unallocated){addBlockRange(ranges,address,length,DataRangeKind::Unallocated);continue;}
            if(entry==Discarded){addBlockRange(ranges,address,length,DataRangeKind::Zero);continue;}
            if(entry>=header.blocksAllocated||!used.insert(entry).second)throw MediaError(DiagnosticCode::VdiMapEntry,{std::to_string(logical)});
            const auto offset=std::uint64_t(header.dataOffset)+std::uint64_t(entry)*(std::uint64_t(header.blockSize)+header.blockExtra)+header.blockExtra;
            if(offset>std::uint64_t(context.length())||std::uint64_t(length)>std::uint64_t(context.length())-offset)throw MediaError(DiagnosticCode::VdiMapOutside,{std::to_string(logical)});
            addBlockRange(ranges,address,length,DataRangeKind::Stored,source,std::int64_t(offset));
        }
        std::optional<HardDiskGeometry> geometry;
        if(header.cylinders&&header.heads&&header.sectors){
            if(header.heads>MaximumSignedField||header.sectors>MaximumSignedField)throw MediaError(DiagnosticCode::VdiGeometry);
            HardDiskGeometry candidate(header.cylinders,std::int32_t(header.heads),std::int32_t(header.sectors),SectorSize);if(candidate.capacity()<=header.diskSize)geometry=std::move(candidate);
        }
        auto blocks=std::make_shared<BlockImage>(header.diskSize,SectorSize,std::move(ranges),std::move(geometry));
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VariantKey),std::string(header.imageType==FixedType?FixedVariant:DynamicVariant)},{std::string(UuidKey),header.imageUuid}});
    });
}
}
