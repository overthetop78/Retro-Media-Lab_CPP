#pragma once
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::MigrationConstants {
inline constexpr char PathSeparator = '/';
inline constexpr std::string_view RootPath = "/";
inline constexpr std::string_view DirectoryKind = "Directory", LinkKind = "Link", UnknownKind = "Unknown";
inline constexpr std::string_view TimestampPattern = "%1-%2-%3T%4:%5:%6.%7%8%9:%10";
inline constexpr int YearWidth = 4, FieldWidth = 2, FractionWidth = 7, NanosecondsPerTick = 100, MinutesPerHour = 60;
inline constexpr char PositiveOffset = '+', NegativeOffset = '-', Padding = '0';
}
