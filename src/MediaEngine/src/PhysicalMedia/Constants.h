#pragma once
#include <cstdint>
#include <array>
#include <utility>
#include <string_view>
namespace RetroMediaLab::MediaEngine::PhysicalDetails {
inline constexpr std::string_view PositionField="position",ProgressField="progress",AttemptField="attempt",OrderField="writeOrder",SamplesField="samples",UnitsField="dataUnits",KindField="mediaKind",PathField="sourcePath",ResolutionField="resolution",DurationField="duration",IntervalField="interval";
inline constexpr int SampleBytes=2,ByteBits=8,ByteMask=255,DecimalBase=10;
inline constexpr std::string_view One="1";
inline constexpr char NameSeparator=',',FirstDigit='0',LastDigit='9';
inline constexpr std::array<std::pair<std::string_view,std::uint8_t>,22> DiskTypes{{
 {"Commodore64",0x00},{"Amiga",0x04},{"AmigaHighDensity",0x08},{"Atari8BitSingleDensity",0x10},{"Atari8BitDoubleDensity",0x11},{"Atari8BitEnhancedDensity",0x12},{"AtariStSingleSided",0x14},{"AtariStDoubleSided",0x15},{"AppleII",0x20},{"AppleIIProDos",0x21},{"AppleMacintosh400",0x24},{"AppleMacintosh800",0x25},{"AppleMacintosh1440",0x26},{"IbmPc360",0x30},{"IbmPc720",0x31},{"IbmPc1200",0x32},{"IbmPc1440",0x33},{"AmstradCpc",0x70},{"Other320",0x80},{"Other1200",0x81},{"Other720",0x84},{"Other1440",0x85}}};
}
