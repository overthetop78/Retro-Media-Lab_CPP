#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Apple::Geometry {
inline constexpr std::int32_t SingleHead=1,DoubleHeads=2,EightyCylinders=80;
namespace AppleII {
inline constexpr std::int32_t SectorSize=256,SectorsPerTrack=16,Dos32SectorsPerTrack=13,TrackCount=35,MaximumReconstructedTrackCount=50;
inline constexpr std::int32_t ProDosBlocksPerTrack=8,SectorsPerProDosBlock=2,ProDosBlockSize=SectorSize*SectorsPerProDosBlock;
inline constexpr std::int32_t TrackSize=SectorSize*SectorsPerTrack,Capacity=TrackCount*TrackSize,Dos32Capacity=TrackCount*Dos32SectorsPerTrack*SectorSize;
inline constexpr std::array<std::int32_t,SectorsPerTrack> ProDosToPhysical{0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15};
inline constexpr std::array<std::int32_t,SectorsPerTrack> PhysicalToDos{0,7,14,6,13,5,12,4,11,3,10,2,9,1,8,15};
}
namespace MacintoshGcr {
inline constexpr std::int32_t BlockSize=512,SingleSidedBlockCount=800,CylinderCount=EightyCylinders;
inline constexpr std::int32_t SingleSidedHeadCount=SingleHead,DoubleSidedHeadCount=DoubleHeads,MaximumSectorsPerTrack=12,ZoneCylinderCount=16;
inline constexpr std::array<std::int32_t,5> ZoneEnds{16,32,48,64,CylinderCount},ZoneSectors{12,11,10,9,8};
inline constexpr std::int32_t Capacity400K=SingleSidedBlockCount*BlockSize,Capacity800K=Capacity400K*DoubleHeads,Capacity1440K=1474560;
}
namespace LisaFileWare {
inline constexpr std::int32_t BlockCount=1702,CylinderCount=46,HeadCount=DoubleHeads,LinearTrackThreshold=84,MaximumSectorsPerTrack=22;
inline constexpr std::array<std::int32_t,8> ZoneEnds{4,11,17,23,29,35,42,CylinderCount},ZoneSectors{22,21,20,19,18,17,16,15};
}
namespace MacintoshMfm {
inline constexpr std::int32_t SectorSize=512,CylinderCount=EightyCylinders,HeadCount=DoubleHeads,SectorsPerTrack=18;
inline constexpr std::int32_t SectorCount=CylinderCount*HeadCount*SectorsPerTrack,Capacity=SectorCount*SectorSize;
}
namespace Tagged {
inline constexpr std::int32_t HeadCount=SingleHead,FirstHead=0,SectorsPerTrack=10,MinimumCylinderCount=1;
}
namespace LayoutNames {
inline constexpr std::string_view D13="D13",AppleII140K="Apple II 140 K",Apple400K="Apple 400 K",Apple800K="Apple 800 K",Macintosh1440K="Macintosh 1.44 M";
}
}
