#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Adf::Constants {
using namespace MediaFileSystems::Formats;
struct Specification {std::string_view id;std::uint32_t blockSize,cylinders,heads,sectorsPerTrack;std::int64_t firstSector;};
inline constexpr std::array<Specification,6> AcornProfiles{{{AcornAdfs160,256,40,1,16,0},{AcornAdfs320,256,80,1,16,0},{AcornAdfs640,256,80,2,16,0},{AcornAdfs800,1024,80,2,5,0},{AcornAdfs1600,1024,80,2,10,0},{AcornDosPlus800,1024,80,2,5,1}}};
inline constexpr std::array<Specification,2> AmigaProfiles{{{AmigaDos,512,80,2,11,0},{AmigaDosHighDensity,512,80,2,22,0}}};
inline constexpr std::array<std::string_view,6> Extensions{{".adf",".ads",".adm",".adl",".640",".800"}};
inline constexpr std::string_view TranslationDomain="MediaEngine";
inline constexpr std::string_view AdfExtension = ".adf";
inline constexpr std::string_view Adfs640Extension = ".640";
inline constexpr std::uint64_t BbcLargeCapacity = 655360;
inline constexpr std::uint64_t Capacity = 819200;
inline constexpr std::uint64_t HighCapacity = 1638400;
inline constexpr std::uint64_t PaddedCapacity = 820224;
inline constexpr std::uint32_t BbcBlockSize = 256;
inline constexpr std::uint32_t ArchimedesBlockSize = 1024;
inline constexpr std::uint32_t ArchimedesSectorLog = 10;
inline constexpr std::uint32_t ProbeLength = 4096;
inline constexpr std::uint32_t OldMapLength = 512;
inline constexpr std::uint8_t DosDescriptor = 0xfd;
inline constexpr std::uint8_t FatHeaderByte = 0xff;
inline constexpr int FatHeaderSize = 3;
inline constexpr int FatHeaderLastByte = 2;
inline constexpr char UpperFirst = 'A';
inline constexpr char UpperLast = 'Z';
inline constexpr int CaseDelta = 'a'-'A';
inline constexpr std::string_view SizeSeparator = ", ";
inline constexpr std::string_view AcornWriterId="acorn-adf",AmigaWriterId="amiga-adf",AdsExtension=".ads",AdmExtension=".adm",AdlExtension=".adl",ImgExtension=".img",DosPlusExtension=".800";
}
