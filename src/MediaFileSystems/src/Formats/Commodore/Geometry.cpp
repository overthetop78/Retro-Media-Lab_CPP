#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <bit>
namespace RetroMediaLab::MediaFileSystems::Commodore {
namespace {
using namespace Constants;
void trackCount(std::int32_t tracks) {
    if(tracks!=StandardTracks&&tracks!=ExtendedTracks)throw MediaError(DiagnosticCode::CommodoreTrackCount,{std::to_string(tracks),std::string(TrackCountsText)});
}
void trackRange(std::int32_t track,std::int32_t maximum) {
    if(track<FirstTrack||track>maximum)throw MediaError(DiagnosticCode::CommodoreTrack,{std::to_string(track),std::to_string(FirstTrack),std::to_string(maximum)});
}
void sectorRange(std::int32_t sector,std::int32_t count) {
    if(sector<0||sector>=count)throw MediaError(DiagnosticCode::CommodoreSector,{std::to_string(sector),std::to_string(count-1)});
}
std::int32_t preceding(std::int32_t track) {
    static const auto offsets=[] {
        std::array<std::int32_t,TrackOffsetSlots> result{};
        for(std::int32_t current=FirstTrack;current<=ExtendedTracks;++current)result[std::size_t(current+1)]=result[std::size_t(current)]+sectors1541(current);
        return result;
    }();
    return offsets[std::size_t(track)];
}
}
std::int32_t sectors1541(std::int32_t track) {
    trackRange(track,Constants::ExtendedTracks);
    for(std::size_t zone=0;zone<Constants::ZoneEnds.size();++zone)if(track<=Constants::ZoneEnds[zone])return Constants::ZoneSectors[zone];
    return Constants::ZoneSectors.back();
}
std::int32_t blocksPerSide1541(std::int32_t tracks) {trackCount(tracks);return preceding(tracks+1);}
std::int32_t sideBlock1541(std::int32_t track,std::int32_t sector,std::int32_t tracks) {
    trackCount(tracks);trackRange(track,tracks);sectorRange(sector,sectors1541(track));return preceding(track)+sector;
}
Address address1541(std::int32_t block,std::int32_t tracks,std::int32_t sides) {
    const auto perSide=blocksPerSide1541(tracks);
    if(sides<=0)throw MediaError(DiagnosticCode::CommodoreSide,{std::to_string(sides),std::to_string(std::int64_t(sides)-1)});
    const auto total=std::bit_cast<std::int32_t>(std::uint32_t(perSide)*std::uint32_t(sides));
    if(block<0||block>=total)throw MediaError(DiagnosticCode::CommodoreBlock,{std::to_string(block),std::to_string(std::int64_t(total)-1)});
    auto remaining=block%perSide;
    for(std::int32_t track=Constants::FirstTrack;track<=tracks;++track) {
        const auto sectors=sectors1541(track);
        if(remaining<sectors)return {track,remaining,block/perSide};
        remaining-=sectors;
    }
    throw MediaError(DiagnosticCode::CommodoreBlock,{std::to_string(block),std::to_string(std::int64_t(total)-1)});
}
std::int32_t cylinder1541(std::int32_t track) noexcept {return std::bit_cast<std::int32_t>(std::uint32_t(track)-1);}
std::int32_t block1571(std::int32_t track,std::int32_t sector,std::int32_t tracks,std::int32_t side) {
    if(side<0||side>=Constants::TwoSides)throw MediaError(DiagnosticCode::CommodoreSide,{std::to_string(side),std::to_string(Constants::TwoSides-1)});
    return side*blocksPerSide1541(tracks)+sideBlock1541(track,sector,tracks);
}
Address address1571(std::int32_t block,std::int32_t tracks) {return address1541(block,tracks,Constants::TwoSides);}
std::int32_t block1581(std::int32_t track,std::int32_t sector) {
    trackRange(track,Constants::Cylinders1581);sectorRange(sector,Constants::BlocksPerTrack1581);return (track-1)*Constants::BlocksPerTrack1581+sector;
}
std::pair<std::int32_t,std::int32_t> address1581(std::int32_t block) {
    const auto count=Constants::Cylinders1581*Constants::BlocksPerTrack1581;
    if(block<0||block>=count)throw MediaError(DiagnosticCode::CommodoreBlock,{std::to_string(block),std::to_string(count-1)});
    return {block/Constants::BlocksPerTrack1581+1,block%Constants::BlocksPerTrack1581};
}
std::int32_t physicalBlock1581(std::int32_t cylinder,std::int32_t head,std::int32_t sector) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(cylinder)*Constants::BlocksPerTrack1581+((std::uint32_t(head)^1)*Constants::PhysicalSectors1581+std::uint32_t(sector)-1)*Constants::BlocksPerPhysicalSector1581);
}
}
