#pragma once
#include <cstdint>
namespace RetroMediaLab::MediaFileSystems::Cpm::AmstradLayoutConstants {
inline constexpr std::int32_t SystemFirstSectorId=0xc1,SystemLastSectorId=0xc9,DataFirstSectorId=0x41,DataLastSectorId=0x49;
inline constexpr std::int32_t CpcDirectoryEntries=64,CpcAllocationSize=1024,CpcDirectoryBlocks=2,CpcSystemReservedTracks=2,CpcDataReservedTracks=0,CpcSectorsPerTrack=9,CpcSectorSize=512;
}
namespace RetroMediaLab::MediaFileSystems::Cpm::AmstradSpecificationConstants {
inline constexpr std::int32_t MinimumLength=512,TracksOffset=2,SectorsPerTrackOffset=3,SectorSizeCodeOffset=4,ReservedTracksOffset=5,AllocationSizeCodeOffset=6,DirectoryBlocksOffset=7;
inline constexpr std::int32_t MaximumTracks=96,MaximumSectorsPerTrack=64,MaximumReservedTracks=8,MaximumDirectoryBlocks=16;
inline constexpr std::int32_t MinimumSectorSize=128,MaximumSectorSize=4096,MinimumAllocationSize=512,MaximumAllocationSize=16384;
inline constexpr std::uint8_t SizeCodeMask=7;
}
