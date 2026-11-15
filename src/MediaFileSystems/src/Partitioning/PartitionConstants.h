#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::PartitionConstants {
inline constexpr std::uint64_t MinimumGptBlockCount = 2;
inline constexpr std::uint64_t FirstExtraEbrEntry = 2;
inline constexpr std::uint64_t BootRecordSize = 512;
inline constexpr std::uint64_t PartitionTableOffset = 446;
inline constexpr std::uint64_t PartitionEntrySize = 16;
inline constexpr std::uint64_t PartitionEntryCount = 4;
inline constexpr std::uint64_t PartitionTypeOffset = 4;
inline constexpr std::uint64_t PartitionLbaOffset = 8;
inline constexpr std::uint64_t PartitionCountOffset = 12;
inline constexpr std::uint64_t BootSignatureOffset = 510;
inline constexpr std::uint64_t BootSignatureFirst = 85;
inline constexpr std::uint64_t BootSignatureLast = 170;
inline constexpr std::uint64_t ProtectiveGptType = 238;
inline constexpr std::uint64_t ExtendedChs = 5;
inline constexpr std::uint64_t ExtendedLba = 15;
inline constexpr std::uint64_t ExtendedLinux = 133;
inline constexpr std::uint64_t MaximumEbrCount = 4096;
inline constexpr std::uint64_t MinimumGptHeaderSize = 92;
inline constexpr std::uint64_t MinimumGptEntrySize = 128;
inline constexpr std::uint64_t MaximumGptArraySize = 134217728;
inline constexpr std::uint64_t GptRevisionOffset = 8;
inline constexpr std::uint64_t GptHeaderSizeOffset = 12;
inline constexpr std::uint64_t GptHeaderCrcOffset = 16;
inline constexpr std::uint64_t GptCurrentLbaOffset = 24;
inline constexpr std::uint64_t GptBackupLbaOffset = 32;
inline constexpr std::uint64_t GptFirstUsableOffset = 40;
inline constexpr std::uint64_t GptLastUsableOffset = 48;
inline constexpr std::uint64_t GptDiskIdOffset = 56;
inline constexpr std::uint64_t GptArrayLbaOffset = 72;
inline constexpr std::uint64_t GptEntryCountOffset = 80;
inline constexpr std::uint64_t GptEntrySizeOffset = 84;
inline constexpr std::uint64_t GptArrayCrcOffset = 88;
inline constexpr std::uint64_t MinimumGptRevision = 65536;
inline constexpr std::uint64_t GptEntryAlignment = 8;
inline constexpr std::uint64_t GptEntryIdOffset = 16;
inline constexpr std::uint64_t GptEntryFirstOffset = 32;
inline constexpr std::uint64_t GptEntryLastOffset = 40;
inline constexpr std::uint64_t GptNameOffset = 56;
inline constexpr std::uint64_t GptNameMaximumBytes = 72;
inline constexpr std::uint64_t HexPrefixLength = 2;
inline constexpr std::uint64_t HexByteLength = 2;
inline constexpr std::array<std::uint8_t, 8> GptSignature{0x45,0x46,0x49,0x20,0x50,0x41,0x52,0x54};
inline constexpr std::string_view HexDigits = "0123456789ABCDEF";
inline constexpr std::string_view HexPrefix = "0x";
inline constexpr std::uint8_t HexShift = 4, HexMask = 15;
}
