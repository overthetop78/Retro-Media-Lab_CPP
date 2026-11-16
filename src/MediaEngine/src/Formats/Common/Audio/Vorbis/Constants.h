#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
namespace RetroMediaLab::MediaEngine::Vorbis::Constants {
inline constexpr int SampleBytes=2;
inline constexpr std::size_t DecodeBytes=65536;
inline constexpr auto MaxSigned=std::numeric_limits<std::int64_t>::max();
inline constexpr auto MaxLong=std::numeric_limits<long>::max();
}
