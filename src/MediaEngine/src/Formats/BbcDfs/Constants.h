#pragma once
#include "BbcDfsImages.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
namespace RetroMediaLab::MediaEngine::BbcDfs::Constants {
inline constexpr std::uint32_t SectorSize=256,SectorsPerTrack=10,TrackSize=SectorSize*SectorsPerTrack,HeaderSectors=2,TwoHeads=2,FortyCylinders=40,EightyCylinders=80;
inline constexpr std::uint64_t SmallSectors=400,LargeSectors=800;
inline constexpr std::string_view WriterId="bbc-dfs";
inline constexpr std::string_view Ssd=".ssd",Dsd=".dsd",Adl=".adl",Ima=".ima",Img=".img";
inline constexpr char UpperFirst='A',UpperLast='Z',CaseDelta='a'-'A';
using namespace MediaFileSystems::Formats;
struct Profile {std::string_view id;std::uint32_t cylinders,heads;};
inline constexpr std::array<Profile,4> Profiles{{{AcornDfsSingleSided,FortyCylinders,1},{AcornDfsSingleSided80,EightyCylinders,1},{AcornDfsDoubleSided,FortyCylinders,TwoHeads},{AcornDfsDoubleSided80,EightyCylinders,TwoHeads}}};
}
