#pragma once
#include <cstdint>
#include <limits>
namespace RetroMediaLab::MediaEngine::Flux::ConversionConstants {
inline constexpr auto MaximumUnsigned=std::numeric_limits<std::uint32_t>::max();
inline constexpr auto MaximumCells=std::numeric_limits<std::int32_t>::max();
}
