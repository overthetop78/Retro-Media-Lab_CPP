#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Commodore::Constants {
inline constexpr std::int32_t TrackOffsetSlots=42,SectorSize=256,FirstTrack=1,StandardTracks=35,ExtendedTracks=40,MaximumSectors=21,TwoSides=2;
inline constexpr std::array<std::int32_t,4> ZoneEnds{17,24,30,ExtendedTracks},ZoneSectors{MaximumSectors,19,18,17};
inline constexpr std::array<std::int32_t,2> TrackCounts{StandardTracks,ExtendedTracks};
inline constexpr std::int32_t Cylinders1581=80,Heads1581=1,BlocksPerTrack1581=40,PhysicalHeads1581=2,PhysicalSectorBytes1581=512,PhysicalSectors1581=10,FirstPhysicalSector1581=1,PhysicalSizeCode1581=2,BlocksPerPhysicalSector1581=2;
inline constexpr std::string_view Format1541="commodore.1541",Format1571="commodore.1571",Format1581="commodore.1581",TrackCountsText="35, 40";
}
