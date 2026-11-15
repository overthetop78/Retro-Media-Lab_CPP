#pragma once
#include <cstdint>
#include <cstddef>
#include <limits>
namespace RetroMediaLab::MediaEngine::Encoding::PackingConstants {
inline constexpr std::size_t BitsPerByte=8,RoundUp=BitsPerByte-1;
inline constexpr std::uint8_t ZeroFill=0,OneFill=0xff;
inline constexpr auto MaximumBitCount=std::numeric_limits<std::int32_t>::max();
}
