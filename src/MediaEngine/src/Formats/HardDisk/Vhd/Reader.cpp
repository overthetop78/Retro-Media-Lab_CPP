#include "Container.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhd {
namespace {
std::vector<DataRange> dynamicRanges(const RecognitionContext& context,std::shared_ptr<const MediaFileSystems::RandomAccessData> source,const Footer& footer,std::stop_token token){
    using namespace Constants;
    if(context.length()<std::int64_t(DynamicHeaderSize)||footer.dataOffset>std::uint64_t(context.length())-DynamicHeaderSize)throw MediaError(DiagnosticCode::VhdHeaderOffset);
    const auto header=context.readAsync(std::int64_t(footer.dataOffset),DynamicHeaderSize,token).get();
    if(!std::equal(DynamicCookie.begin(),DynamicCookie.end(),header.begin()))throw MediaError(DiagnosticCode::VhdDynamicCookie);
    if(qFromBigEndian<std::uint32_t>(header.data()+DynamicChecksumOffset)!=checksum(header,DynamicChecksumOffset))throw MediaError(DiagnosticCode::VhdDynamicChecksum);
    const auto offset=qFromBigEndian<std::uint64_t>(header.data()+TableOffset);if(offset>MaximumLogicalSize)throw MediaError(DiagnosticCode::VhdTableOffset);
    const auto maximum=qFromBigEndian<std::uint32_t>(header.data()+MaximumEntriesOffset),block=qFromBigEndian<std::uint32_t>(header.data()+BlockSizeOffset);
    if(!block||block%SectorSize)throw MediaError(DiagnosticCode::VhdBlockSize);
    const auto count=(std::uint64_t(footer.currentSize)+block-1)/block;
    if(maximum<count||count>MaximumSignedField)throw MediaError(DiagnosticCode::VhdTableCoverage);
    const auto length=count*EntryBytes;
    if(length>MaximumSignedField||length>std::uint64_t(context.length())||offset>std::uint64_t(context.length())-length)throw MediaError(DiagnosticCode::VhdTableRange);
    const auto table=context.readAsync(std::int64_t(offset),std::int32_t(length),token).get();
    const auto sectorsPerBlock=block/SectorSize,bitmapBytes=(sectorsPerBlock+ByteBits-1)/ByteBits,bitmapStorage=(bitmapBytes+SectorSize-1)/SectorSize*SectorSize;
    std::vector<DataRange> ranges;
    for(std::uint64_t i=0;i<count;++i){checkCancellation(token);
        const auto address=std::int64_t(i*block),logicalLength=std::min(std::int64_t(block),footer.currentSize-address);const auto entry=qFromBigEndian<std::uint32_t>(table.data()+i*EntryBytes);
        if(entry==UnusedEntry){addBlockRange(ranges,address,logicalLength,DataRangeKind::Unallocated,{},0,true);continue;}
        const auto stored=std::uint64_t(entry)*SectorSize,data=stored+bitmapStorage;
        if(data>std::uint64_t(context.length())||std::uint64_t(logicalLength)>std::uint64_t(context.length())-data)throw MediaError(DiagnosticCode::VhdEntryRange,{std::to_string(i)});
        const auto bitmap=context.readAsync(std::int64_t(stored),std::int32_t(bitmapBytes),token).get();const auto sectors=(std::uint64_t(logicalLength)+SectorSize-1)/SectorSize;
        for(std::uint64_t sector=0;sector<sectors;++sector){checkCancellation(token);const auto logical=address+std::int64_t(sector*SectorSize),size=std::min(std::int64_t(SectorSize),footer.currentSize-logical);
            if(bitmap[std::size_t(sector/ByteBits)]&(BitmapFirstBit>>(sector%ByteBits)))addBlockRange(ranges,logical,size,DataRangeKind::Stored,source,std::int64_t(data+sector*SectorSize),true);
            else addBlockRange(ranges,logical,size,DataRangeKind::Zero,{},0,true);
        }
    }
    return ranges;
}
}
const ReaderCapabilities& Reader::capabilities()const{
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{FooterCookie.begin(),FooterCookie.end()}},{},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
        try{const auto footer=readFooter(context,token);return footer.diskType==Constants::FixedType||footer.diskType==Constants::DynamicType;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto footer=readFooter(context,token);const auto& path=context.source()->primaryPath();const auto source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));
        if(source->length()!=std::uint64_t(context.length()))throw MediaError(DiagnosticCode::VhdChanged);
        std::vector<DataRange> ranges;
        if(footer.diskType==FixedType){if(footer.currentSize!=context.length()-std::int64_t(FooterSize))throw MediaError(DiagnosticCode::VhdPayload);ranges.emplace_back(0,footer.currentSize,DataRangeKind::Stored,source);}
        else if(footer.diskType==DynamicType)ranges=dynamicRanges(context,source,footer,token);
        else if(footer.diskType==DifferencingType)throw MediaError(DiagnosticCode::VhdParent);
        else throw MediaError(DiagnosticCode::VhdType,{std::to_string(footer.diskType)});
        std::optional<HardDiskGeometry> geometry;if(footer.cylinders&&footer.heads&&footer.sectors){HardDiskGeometry candidate(footer.cylinders,footer.heads,footer.sectors,SectorSize);if(candidate.capacity()<=footer.currentSize)geometry=std::move(candidate);}
        auto blocks=std::make_shared<BlockImage>(footer.currentSize,std::int32_t(SectorSize),std::move(ranges),std::move(geometry));
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VariantKey),std::string(footer.diskType==FixedType?FixedVariant:DynamicVariant)},{std::string(UuidKey),footer.uuid}});
    });
}
}
