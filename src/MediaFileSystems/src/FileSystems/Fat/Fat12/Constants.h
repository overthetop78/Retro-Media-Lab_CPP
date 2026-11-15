#pragma once
#include "../Constants.h"
namespace RetroMediaLab::MediaFileSystems::Fat12Constants {
namespace Fat12Table {
inline constexpr std::uint32_t FirstDataCluster = 2;
inline constexpr std::uint32_t FreeCluster = 0x000;
inline constexpr std::uint32_t FirstReservedCluster = 0xff0;
inline constexpr std::uint32_t LastReservedCluster = 0xff6;
inline constexpr std::uint32_t BadCluster = 0xff7;
inline constexpr std::uint32_t FirstEndOfChain = 0xff8;
inline constexpr std::uint32_t LastEndOfChain = 0xfff;
inline constexpr std::uint8_t MinimumMediaDescriptor = 0xf0;
inline constexpr std::uint8_t ReservedEntryByte = 0xff;
}
namespace Fat12Layout {
inline constexpr std::uint32_t MaximumClusterCount = 4085;
}
namespace Fat12DirectoryReader {
inline constexpr std::uint32_t MaximumDepth = 64;
}
namespace Values {
inline constexpr std::uint32_t SpecialDirectoryEntries = 2, DoubleSectorCluster = 2, BootJumpPaddingIndex = 2, BootSignatureOffset = 510;
inline constexpr std::uint8_t BootSignatureLow = 0x55, BootSignatureHigh = 0xAA;
inline constexpr std::uint32_t FatHeaderLength = 3, LastHeaderByte = 2;
inline constexpr std::string_view SectorRangeName = "sector range";
inline constexpr std::uint32_t PackedPairDenominator = 2, PackedPairNumerator = 3, NibbleShift = 4;
inline constexpr std::uint8_t LowNibble = 0x0F, HighNibble = 0xF0;
inline constexpr std::uint32_t DosPlusSectorSize = 1024, DosPlusSectorCount = 800, DosPlusFatSectors = 2, DosPlusRootSectors = 6;
inline constexpr std::uint8_t DosPlusDescriptor = 0xFD;
inline constexpr std::uint32_t LegacyReservedCount = 1, LegacyFatCopies = 2, HistoricalTableFatCount = 1;
inline constexpr std::string_view FileSystemId = "fat12", AllowedPunctuation = "$%'-_@~`!(){}^#&";
inline constexpr std::uint32_t SmallWriterSectors = 360, MediumWriterSectors = 1440, LargeWriterSectors = 2880;
inline constexpr std::uint32_t SmallRootEntries = 64, MediumRootEntries = 112, LargeRootEntries = 224, WriterFatCopies = 2;
inline constexpr std::uint8_t HighDensityMedia = 0xF0, DoubleDensityMedia = 0xF9, JumpDisplacement = 0x3C, JumpPadding = 0x90;
inline constexpr std::string_view OemMsx = "MSX     ", OemRetroMediaLab = "RetroMediaLab   ";
inline constexpr std::uint32_t MinimumFatYear = 1980, MaximumFatYear = 2107;
inline constexpr std::uint32_t FileSizeMaximum = 0xFFFFFFFF;
inline constexpr std::uint64_t MaximumDecodedBytes = 65535 * 512;
inline constexpr std::uint32_t MaximumWriterSectors = 65535;
}
}
