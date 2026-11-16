#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AppleScp::Constants {
inline constexpr std::string_view MacintoshName="Macintosh/Lisa",AppleIIName="Apple II",ProDosName="Apple II ProDOS",RwtsName="RWTS18",MacSectorName="Apple Macintosh",RwtsSectorName="Apple II RWTS18";
inline constexpr std::array CellFactors{1.0,0.95,1.05,0.9,1.1,0.85,1.15};
inline constexpr std::int32_t DistinctWeight=100,ValidWeight=10,CellMultiplier=2;
inline constexpr std::size_t LisaTagMinimum=6,LisaTagHigh=4,LisaTagLow=5;
inline constexpr std::uint8_t LisaTagHighValue=0,LisaTagLowValue=1;
inline constexpr std::string_view FailureSeparator="; ";
}
