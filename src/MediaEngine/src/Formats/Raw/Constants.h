#pragma once
#include "RawImages.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
namespace RetroMediaLab::MediaEngine::Raw::Constants {
inline constexpr std::string_view IbmWriterId="ibm-raw",MsxWriterId="msx-raw";
inline constexpr std::string_view UcsdWriterId="ucsd";
inline constexpr std::string_view Bin=".bin";
inline constexpr std::uint32_t UcsdCylinders=40,UcsdHeads=1,UcsdSectors=8;
inline constexpr std::uint32_t SectorSize=512,BytesPerKibibyte=1024,OemOffset=3,OemLength=8,DescriptorOffset=21;
inline constexpr std::uint64_t Ibm160Capacity=160*1024,Ibm180Capacity=180*1024,Ibm320Capacity=320*1024,Ibm360Capacity=360*1024;
inline constexpr std::uint64_t AmbiguousMsxCapacity=368640;
inline constexpr std::uint8_t MsxOneDoubleDensity=0xF8,Ibm160Descriptor=0xFE,Ibm180Descriptor=0xFC,Ibm320Descriptor=0xFF,Ibm360Descriptor=0xFD;
inline constexpr std::string_view Ima=".ima",Img=".img",Dsk=".dsk",MsxPrefix="msx.";
inline constexpr std::string_view IbmPrefix="ibm.",MsxOem="MSX";
inline constexpr char UpperFirst='A',UpperLast='Z',CaseDelta='a'-'A';
struct Profile {std::string_view id;std::uint32_t cylinders,heads,sectors;};
using namespace MediaFileSystems::Formats;
inline constexpr std::array<Profile,11> Ibm{{{Ibm160,40,1,8},{Ibm180,40,1,9},{Ibm320,40,2,8},{Ibm360,40,2,9},{Ibm720,80,2,9},{Ibm800,80,2,10},{Ibm1200,80,2,15},{Ibm1440,80,2,18},{Ibm1680,80,2,21},{IbmDmf,80,2,21},{Ibm2880,80,2,36}}};
inline constexpr std::array<Profile,4> Msx{{{Msx1D,40,1,9},{Msx1Dd,80,1,9},{Msx2D,40,2,9},{Msx2Dd,80,2,9}}};
}
