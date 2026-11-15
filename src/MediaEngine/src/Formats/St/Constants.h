#pragma once
#include "../FormatIds.h"
#include <array>
#include <limits>
namespace RetroMediaLab::MediaEngine::St::Constants {
inline constexpr std::uint32_t SectorSize=512,BytesPerKibibyte=1024;
inline constexpr std::uint32_t MaximumBpbSectors=36,MinimumFallbackCylinders=35,MaximumFallbackCylinders=90;
inline constexpr std::array<std::uint32_t,4> FallbackSectors{9,10,11,18};
inline constexpr std::array<std::uint32_t,2> FallbackHeads{2,1};
inline constexpr auto MaxCapacity=std::numeric_limits<std::int32_t>::max();
inline constexpr std::string_view CylindersField="cylinders",HeadsField="heads",SectorsField="sectorsPerTrack",CapacityField="capacity";
struct Profile {std::string_view id;std::int32_t cylinders,heads,sectors;};
using namespace Formats;
inline constexpr std::array<Profile,9> Profiles{{{AtariSt180,40,1,9},{AtariSt360,40,2,9},{AtariSt400,80,1,10},{AtariSt440,80,1,11},{AtariSt720,80,2,9},{AtariSt800,80,2,10},{AtariSt810,90,2,9},{AtariSt880,80,2,11},{AtariSt1440,80,2,18}}};
inline constexpr std::uint32_t FirstSector=1;
inline constexpr char GeometrySeparator='x';
}
