#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems {
namespace FatBootSectorLayout {
inline constexpr std::uint8_t ShortJumpOpcode = 0xEB;
inline constexpr std::uint8_t NearJumpOpcode = 0xE9;
inline constexpr std::uint32_t UInt16Length = sizeof(std::uint16_t);
inline constexpr std::uint32_t UInt32Length = sizeof(std::uint32_t);
inline constexpr std::uint32_t MinimumLength = 36;
inline constexpr std::uint32_t SectorSize = 512;
inline constexpr std::uint32_t OemOffset = 3;
inline constexpr std::uint32_t OemLength = 8;
inline constexpr char NullPadding = '\0';
inline constexpr char SpacePadding = ' ';
inline constexpr std::uint32_t FirstSectorNumber = 1;
inline constexpr std::uint32_t BootLogicalBlock = 0;
inline constexpr std::uint32_t FirstFatLogicalBlock = 1;
inline constexpr std::uint32_t SystemCylinder = 0;
inline constexpr std::uint32_t SystemHead = 0;
inline constexpr std::uint32_t BootSectorNumber = FirstSectorNumber;
inline constexpr std::uint32_t FirstFatSectorNumber = BootSectorNumber + 1;
inline constexpr std::uint32_t FatMediaDescriptorDataOffset = 0;
inline constexpr std::uint8_t UnknownMediaDescriptor = 0;
inline constexpr std::uint32_t MaximumSectorsPerTrack = 63;
inline constexpr std::uint32_t MaximumHeadCount = 2;
inline constexpr std::uint32_t MaximumCylinderCount = 255;
inline constexpr std::uint32_t BytesPerSectorOffset = 11;
inline constexpr std::uint32_t SectorsPerClusterOffset = 13;
inline constexpr std::uint32_t ReservedSectorCountOffset = 14;
inline constexpr std::uint32_t FatCountOffset = 16;
inline constexpr std::uint32_t RootEntryCountOffset = 17;
inline constexpr std::uint32_t TotalSectors16Offset = 19;
inline constexpr std::uint32_t MediaDescriptorOffset = 21;
inline constexpr std::uint32_t SectorsPerFatOffset = 22;
inline constexpr std::uint32_t SectorsPerTrackOffset = 24;
inline constexpr std::uint32_t HeadCountOffset = 26;
inline constexpr std::uint32_t TotalSectors32Offset = 32;
inline constexpr std::uint32_t VolumeLabelOffset = 43;
inline constexpr std::uint32_t VolumeLabelLength = 11;
inline constexpr std::uint32_t ExtendedBootMinimumLength = VolumeLabelOffset + VolumeLabelLength;
inline constexpr std::string_view EmptyVolumeLabel = "NO NAME";
inline constexpr std::uint8_t PrintableAsciiStart = 0x20;
inline constexpr std::uint8_t PrintableAsciiEnd = 0x7e;
}
}
