#pragma once
#include <array>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include "Constants.h"
namespace RetroMediaLab::MediaFileSystems::Fat12Formats {
using namespace Formats;
inline constexpr std::array<std::string_view, 28> Supported{AcornDosPlus800,Jfd,AtariSt180,AtariSt360,AtariSt400,AtariSt440,AtariSt720,AtariSt800,AtariSt810,AtariSt880,AtariSt1440,Ibm160,Ibm180,Ibm320,Ibm360,Ibm720,Ibm800,Ibm1200,Ibm1440,Ibm1680,IbmDmf,Ibm2880,IbmScan,Msx1D,Msx1Dd,Msx2D,Msx2Dd,ApricotPcXi315};
struct LegacyLayout { std::string_view formatId; std::uint32_t totalSectors, sectorsPerCluster, rootEntries, sectorsPerFat; };
inline constexpr std::array<LegacyLayout, 5> Legacy{{{Ibm160,320,1,64,1},{Ibm180,360,1,64,2},{Ibm320,640,2,112,1},{Ibm360,720,2,112,2},{ApricotPcXi315,630,1,64,2}}};
}
