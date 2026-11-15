#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::MetadataConstants {
inline constexpr int MonthsPerYear = 12, MaximumMonthDays = 31;
inline constexpr int MinimumYear = 1, MaximumYear = 9999, HoursPerDay = 24, MinutesPerHour = 60, SecondsPerMinute = 60;
inline constexpr int NanosecondsPerSecond = 1000000000, MaximumOffsetMinutes = 14 * 60;
inline constexpr std::string_view RootPath = "/", Empty = "";
inline constexpr char PathSeparator = '/', AlternatePathSeparator = '\\', ExtensionSeparator = '.';
inline constexpr char UpperFirst = 'A', UpperLast = 'Z', LowerFirst = 'a', LowerLast = 'z', DigitFirst = '0', DigitLast = '9';
inline constexpr int CaseDelta = 'a' - 'A';
inline constexpr std::uint32_t AsciiLimit = 128;
inline constexpr std::string_view FormatField = "formatId", BlockSizeField = "blockSize", CylindersField = "cylinders", HeadsField = "heads", TrackSectorsField = "sectorsPerTrack", CountField = "logicalBlockCount", CapacityField = "capacity";
}
