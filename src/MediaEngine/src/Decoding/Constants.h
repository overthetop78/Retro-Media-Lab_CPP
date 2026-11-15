#pragma once
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Decoding::Constants {
inline constexpr char PlaceholderStart='{',PlaceholderEnd='}',DigitFirst='0',DigitLast='9';
inline constexpr std::size_t DecimalBase=10;
inline constexpr int SectorWeight=2;
inline constexpr double StandardDivisor=20.0,MaximumConfidence=1.0;
}
