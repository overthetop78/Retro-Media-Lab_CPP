#pragma once
#include <cstdint>
#include <string_view>
#include <array>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
namespace RetroMediaLab::MediaFileSystems::BbcDfs::Constants {
inline constexpr std::uint32_t SectorSize = 256;
inline constexpr std::uint32_t SectorsPerTrack = 10;
inline constexpr std::uint32_t NamesSector = 0;
inline constexpr std::uint32_t MetadataSector = 1;
inline constexpr std::uint32_t MaximumEntryCount = 31;
inline constexpr std::uint32_t EntryPartSize = 8;
inline constexpr std::uint32_t TitleFirstOffset = 0;
inline constexpr std::uint32_t TitleFirstLength = 8;
inline constexpr std::uint32_t TitleSecondLength = 4;
inline constexpr std::uint32_t EntryCountOffset = 5;
inline constexpr std::uint32_t TotalSectorsHighOffset = 6;
inline constexpr std::uint32_t TotalSectorsLowOffset = 7;
inline constexpr std::uint8_t TotalSectorsHighMask = 3;
inline constexpr std::uint32_t FirstEntryOffset = 8;
inline constexpr std::uint32_t LeafNameLength = 7;
inline constexpr std::uint32_t DirectoryOffset = 7;
inline constexpr std::uint8_t CharacterMask = 0x7f;
inline constexpr std::uint8_t LockedBit = 0x80;
inline constexpr std::uint32_t LengthOffset = 4;
inline constexpr std::uint32_t PackedOffset = 6;
inline constexpr std::uint32_t StartSectorOffset = 7;
inline constexpr std::uint8_t LengthHighMask = 0x30;
inline constexpr std::uint32_t LengthHighShift = 12;
inline constexpr std::uint8_t StartSectorHighMask = 3;
inline constexpr std::uint8_t LoadHighMask = 0x0c;
inline constexpr std::uint32_t LoadHighShift = 14;
inline constexpr std::uint8_t ExecuteHighMask = 0xc0;
inline constexpr std::uint32_t ExecuteHighShift = 10;
inline constexpr char RootDirectory = '$';
inline constexpr std::uint32_t BitsPerByte=8,TwoHeads=2,FortyCylinders=40,SmallSectorCount=400,LargeSectorCount=800,CatalogueSectors=2,HexWidth=6,ExecuteOffset=2,ExecuteHighByte=3;
inline constexpr char HexPadding='0';
inline constexpr char Space=' ',Null='\0',Dot='.',UpperFirst='A',UpperLast='Z',LowerFirst='a',LowerLast='z',CaseDelta='a'-'A';
inline constexpr std::uint8_t PrintableFirst=32,PrintableLast=126;
inline constexpr std::string_view SideGeometry="side geometry";
inline constexpr std::string_view FileSystemId="acorn-dfs",LoadPrefix="DFS load &",ExecutePrefix=", execute &";
using namespace Formats;
inline constexpr std::array Supported{AcornAtomDos,Jfd,AcornDfsSingleSided,AcornDfsSingleSided80,AcornDfsDoubleSided,AcornDfsDoubleSided80};
}
