#include "Writing.h"
#include <QtEndian>
#include <QUuid>
#include <chrono>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhd {
Geometry calculateGeometry(std::int64_t total){
    using namespace Constants;const auto sectors=std::min(total,MaximumCylinders*MaximumHeads*MaximumSectors);std::int64_t perTrack{},heads{},cylindersTimesHeads{};
    if(sectors>=MaximumCylinders*MaximumHeads*GeometryThresholdSectors){perTrack=MaximumSectors;heads=MaximumHeads;cylindersTimesHeads=sectors/perTrack;}
    else{perTrack=InitialSectors;cylindersTimesHeads=sectors/perTrack;heads=std::max((cylindersTimesHeads+CylinderThreshold-1)/CylinderThreshold,MinimumHeads);
        if(cylindersTimesHeads>=heads*CylinderThreshold||heads>MaximumHeads){perTrack=MiddleSectors;heads=MaximumHeads;cylindersTimesHeads=sectors/perTrack;}
        if(cylindersTimesHeads>=heads*CylinderThreshold){perTrack=GeometryThresholdSectors;heads=MaximumHeads;cylindersTimesHeads=sectors/perTrack;}
    }
    return {std::uint16_t(std::min(cylindersTimesHeads/heads,MaximumCylinders)),std::uint8_t(heads),std::uint8_t(perTrack)};
}
std::vector<std::uint8_t> createFooter(const BlockImage& blocks,std::uint32_t diskType,std::uint64_t dataOffset){
    using namespace Constants;std::vector<std::uint8_t> footer(FooterSize);std::copy(FooterCookie.begin(),FooterCookie.end(),footer.begin());
    const auto put=[&](std::size_t offset,std::uint32_t value){qToBigEndian(value,footer.data()+offset);};
    put(FeaturesOffset,FooterFeatures);put(VersionOffset,Version);qToBigEndian(dataOffset,footer.data()+DataOffset);
    const auto seconds=std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()-EpochUnixSeconds;put(TimestampOffset,std::uint32_t(std::clamp(seconds,std::int64_t(0),std::int64_t(MaximumUnsignedField))));
    std::copy(Creator.begin(),Creator.end(),footer.begin()+CreatorOffset);put(CreatorVersionOffset,Version);put(CreatorHostOffset,CreatorHostOs);qToBigEndian(std::uint64_t(blocks.capacity()),footer.data()+OriginalSizeOffset);qToBigEndian(std::uint64_t(blocks.capacity()),footer.data()+CurrentSizeOffset);
    const auto geometry=calculateGeometry(blocks.capacity()/SectorSize);qToBigEndian(geometry.cylinders,footer.data()+CylindersOffset);footer[HeadsOffset]=geometry.heads;footer[SectorsOffset]=geometry.sectors;put(TypeOffset,diskType);
    const auto id=QUuid::createUuid().toRfc4122();std::copy(id.begin(),id.end(),footer.begin()+UuidOffset);put(ChecksumOffset,checksum(footer,ChecksumOffset));return footer;
}
std::vector<std::uint8_t> createDynamicHeader(std::uint32_t count){
    using namespace Constants;std::vector<std::uint8_t> header(DynamicHeaderSize);std::copy(DynamicCookie.begin(),DynamicCookie.end(),header.begin());qToBigEndian(NoDataOffset,header.data()+DynamicNextOffset);qToBigEndian(std::uint64_t(DynamicTableOffset),header.data()+TableOffset);qToBigEndian(Version,header.data()+DynamicVersionOffset);qToBigEndian(count,header.data()+MaximumEntriesOffset);qToBigEndian(std::uint32_t(DefaultBlockSize),header.data()+BlockSizeOffset);qToBigEndian(checksum(header,DynamicChecksumOffset),header.data()+DynamicChecksumOffset);return header;
}
}
