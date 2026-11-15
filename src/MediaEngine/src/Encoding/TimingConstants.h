#pragma once
#include <cstdint>

namespace RetroMediaLab::MediaEngine::Encoding::TimingConstants {
inline constexpr std::uint32_t HighDensityMfmCell = 40, DoubleDensityMfmCell = 80;
inline constexpr std::uint32_t ExtraDensityMfmCell = 20, SingleDensityFmCell = 160;
inline constexpr std::uint32_t Rpm300Index = 8000000, Rpm360Index = 6666667;
}
