#pragma once
#include <limits>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::SectorConstants {
inline constexpr std::int32_t MinimumIsoSectorSize=128,MaximumIsoSizeCode=7;
inline constexpr char UpperFirst='A', UpperLast='Z', CaseDelta='a'-'A';
inline constexpr std::string_view GeometrySeparator=" / ";
inline constexpr std::string_view SectorNumberField="firstSectorNumber";
inline constexpr auto CapacityLimit = std::uint64_t(std::numeric_limits<std::int64_t>::max());
inline constexpr std::string_view FormatField = "formatId", BlockSizeField = "blockSize", CylindersField = "cylinders", HeadsField = "heads", TrackSectorsField = "sectorsPerTrack", CountField = "logicalBlockCount", CapacityField = "capacity";
inline constexpr std::string_view GeometryExpression = "cylinders*heads*sectorsPerTrack", Overflow = "overflow";
}
