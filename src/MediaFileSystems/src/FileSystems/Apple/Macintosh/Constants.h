#pragma once
#include <cstdint>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Macintosh::Constants {
inline constexpr std::uint8_t AsciiMaximum=0x7f,AsciiReplacement='?',FirstAsciiWhitespace=0x09,LastAsciiWhitespace=0x0d;
inline constexpr char Space=' ',Null='\0';
inline constexpr std::int32_t EpochYear=1904,EpochMonth=1,EpochDay=1,MinimumYear=1,MaximumYear=9999;
inline constexpr char16_t NativeSeparator=u':',PathSeparator=u'/';
inline constexpr std::uint32_t MaximumReference=std::uint32_t(std::numeric_limits<std::int32_t>::max());
}
