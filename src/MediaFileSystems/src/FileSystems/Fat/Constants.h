#pragma once
#include <cstdint>
#include <RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::FatConstants {
namespace FatBootSectorLayout = RetroMediaLab::MediaFileSystems::FatBootSectorLayout;
namespace FatDirectoryLayout {
inline constexpr std::uint8_t ReadOnlyAttribute = 1;
inline constexpr std::uint8_t HiddenAttribute = 2;
inline constexpr std::uint8_t SystemAttribute = 4;
inline constexpr std::uint8_t VolumeLabelAttribute = 8;
inline constexpr std::uint8_t DirectoryAttribute = 16;
inline constexpr std::uint8_t ArchiveAttribute = 32;
inline constexpr std::uint32_t EntrySize = 32;
inline constexpr std::uint32_t NameLength = 8;
inline constexpr std::uint32_t ExtensionOffset = NameLength;
inline constexpr std::uint32_t ExtensionLength = 3;
inline constexpr std::uint32_t AttributesOffset = 11;
inline constexpr std::uint32_t ModifiedTimeOffset = 22;
inline constexpr std::uint32_t ModifiedDateOffset = 24;
inline constexpr std::uint32_t FirstClusterOffset = 26;
inline constexpr std::uint32_t FileSizeOffset = 28;
inline constexpr std::uint8_t EndMarker = 0x00;
inline constexpr std::uint8_t DeletedMarker = 0xe5;
inline constexpr std::uint8_t LongFileName = ReadOnlyAttribute | HiddenAttribute | SystemAttribute | VolumeLabelAttribute;
inline constexpr std::string_view CurrentDirectoryName = ".";
inline constexpr std::string_view ParentDirectoryName = "..";
inline constexpr char ExtensionSeparator = '.';
}
namespace FatDateTime {
inline constexpr std::uint32_t BaseYear = 1980;
inline constexpr std::uint32_t YearShift = 9;
inline constexpr std::uint32_t MonthShift = 5;
inline constexpr std::uint32_t MonthMask = 0x0f;
inline constexpr std::uint32_t DayMask = 0x1f;
inline constexpr std::uint32_t HourShift = 11;
inline constexpr std::uint32_t MinuteShift = 5;
inline constexpr std::uint32_t MinuteMask = 0x3f;
inline constexpr std::uint32_t SecondMask = 0x1f;
inline constexpr std::uint32_t SecondMultiplier = 2;
}
namespace Text {
inline constexpr char Null = '\0', Space = ' ', Dot = '.', PathSeparator = '/', AlternatePathSeparator = '\\';
inline constexpr std::uint32_t LowerAsciiLimit = 128;
}
}
