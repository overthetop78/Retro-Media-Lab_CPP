#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
namespace RetroMediaLab::MediaFileSystems::Composition::CatalogConstants {
inline constexpr std::string_view Reader0=FileSystemIds::AmigaDos;
inline constexpr std::array<std::string_view,2> Formats0{Formats::AmigaDos,Formats::AmigaDosHighDensity};
inline constexpr std::string_view Reader1=FileSystemIds::AmigaFlatResourceArchive;
inline constexpr std::array<std::string_view,2> Formats1{Formats::AmigaDos,Formats::AmigaDosHighDensity};
inline constexpr std::string_view Reader2=FileSystemIds::AcornAdfs;
inline constexpr std::array<std::string_view,3> Formats2{Formats::AcornAdfs800,Formats::AcornAdfs1600,Formats::Jfd};
inline constexpr std::string_view Reader3=FileSystemIds::AcornDfs;
inline constexpr std::array<std::string_view,6> Formats3{Formats::AcornAtomDos,Formats::Jfd,Formats::AcornDfsSingleSided,Formats::AcornDfsSingleSided80,Formats::AcornDfsDoubleSided,Formats::AcornDfsDoubleSided80};
inline constexpr std::string_view Reader4=FileSystemIds::Coherent;
inline constexpr std::array<std::string_view,1> Formats4{Formats::Commodore900Coherent};
inline constexpr std::string_view Reader5=FileSystemIds::Rt11;
inline constexpr std::array<std::string_view,1> Formats5{Formats::DecRx02};
inline constexpr std::string_view Reader6=FileSystemIds::Ucsd;
inline constexpr std::array<std::string_view,1> Formats6{Formats::UcsdIbmMfm};
inline constexpr std::string_view Reader7=FileSystemIds::AppleDos;
inline constexpr std::array<std::string_view,3> Formats7{Formats::AppleIIDos32,Formats::AppleIIDos33,Formats::AppleIIAppleDos140};
inline constexpr std::string_view Reader8=FileSystemIds::ProDos;
inline constexpr std::array<std::string_view,4> Formats8{Formats::AppleIIProDos,Formats::AppleIIProDos140,Formats::AppleIIProDos800,Formats::AppleIIISos};
inline constexpr std::string_view Reader9=FileSystemIds::MacMfs;
inline constexpr std::array<std::string_view,4> Formats9{Formats::AppleMacMfs,Formats::Mac400,Formats::Mac800,Formats::Mac1440};
inline constexpr std::string_view Reader10=FileSystemIds::MacHfs;
inline constexpr std::array<std::string_view,4> Formats10{Formats::AppleMacHfs,Formats::Mac400,Formats::Mac800,Formats::Mac1440};
inline constexpr std::string_view Reader11=FileSystemIds::Lisa;
inline constexpr std::array<std::string_view,1> Formats11{Formats::AppleLisaOffice};
inline constexpr std::string_view Reader12=FileSystemIds::AmstradCpm;
inline constexpr std::string_view Reader13=FileSystemIds::Cpm;
inline constexpr std::string_view Reader14=FileSystemIds::CommodoreDos;
inline constexpr std::array<std::string_view,3> Formats14{Formats::Commodore1541,Formats::Commodore1571,Formats::Commodore1581};
inline constexpr std::string_view Reader15=FileSystemIds::Fat12;
inline constexpr std::array<std::string_view,28> Formats15{Formats::AcornDosPlus800,Formats::Jfd,Formats::AtariSt180,Formats::AtariSt360,Formats::AtariSt400,Formats::AtariSt440,Formats::AtariSt720,Formats::AtariSt800,Formats::AtariSt810,Formats::AtariSt880,Formats::AtariSt1440,Formats::Ibm160,Formats::Ibm180,Formats::Ibm320,Formats::Ibm360,Formats::Ibm720,Formats::Ibm800,Formats::Ibm1200,Formats::Ibm1440,Formats::Ibm1680,Formats::IbmDmf,Formats::Ibm2880,Formats::IbmScan,Formats::Msx1D,Formats::Msx1Dd,Formats::Msx2D,Formats::Msx2Dd,Formats::ApricotPcXi315};
inline constexpr std::string_view Reader16=FileSystemIds::AtariSpartaDos;
inline constexpr std::array<std::string_view,9> Formats16{Formats::Atari90,Formats::Atari130,Formats::Atari140,Formats::Atari180,Formats::AtariAtx,Formats::AtariXfd90,Formats::AtariXfd130,Formats::AtariXfd140,Formats::AtariXfd180};
inline constexpr std::string_view Reader17=FileSystemIds::AtariDos;
inline constexpr std::array<std::string_view,9> Formats17{Formats::Atari90,Formats::Atari130,Formats::Atari140,Formats::Atari180,Formats::AtariAtx,Formats::AtariXfd90,Formats::AtariXfd130,Formats::AtariXfd140,Formats::AtariXfd180};
inline constexpr std::string_view Reader18=FileSystemIds::AtariKFile;
inline constexpr std::string_view Reader19=FileSystemIds::AtariClkGraphicsLibrary;
inline constexpr std::string_view Reader20=FileSystemIds::AtariBootDisk;
}
