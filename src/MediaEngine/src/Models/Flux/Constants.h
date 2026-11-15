#pragma once
#include <cstdint>
#include <limits>
namespace RetroMediaLab::MediaEngine::Flux::Constants {
inline constexpr double MinimumCell=1.0,FallbackCell=1.0,MinimumSampleRatio=0.7,MaximumSampleRatio=1.3,Adaptation=0.08;
inline constexpr double MinimumPllRatio=0.9,MaximumPllRatio=1.1,HalfCycle=0.5,PhaseRetention=0.4,PllCorrection=0.05,RobustDivisor=2.0;
inline constexpr int MaximumFmMfmCells=32,MaximumNrziCells=64,EstimatedBits=4,MaximumDirectZeros=3,PercentileDivisor=50,LowerClusterDivisor=5,MedianDivisor=2;
inline constexpr int MinimumCells=1,BitsPerByte=8,UshortPatternBits=16,MinimumPatternBits=1,MaximumUintPatternBits=32,MfmCellsPerDataBit=2,FmCellsPerDataBit=4;
inline constexpr std::uint64_t MaximumBits=2147483647;
inline constexpr std::uint32_t MaximumInterval=std::numeric_limits<std::uint32_t>::max();
}
