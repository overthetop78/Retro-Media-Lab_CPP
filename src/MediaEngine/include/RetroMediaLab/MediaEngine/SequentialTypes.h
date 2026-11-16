#pragma once
#include <chrono>
#include <cstdint>
namespace RetroMediaLab::MediaEngine {
enum class SequentialSegmentKind {Unknown,Samples,Pulse,Carrier,Silence,DataBlock,Record,TapeMark};
enum class SequentialTravelDirection {Unknown,Forward,Reverse};
inline constexpr std::intmax_t SequentialTicksPerSecond=10000000;
using SequentialTime=std::chrono::duration<std::int64_t,std::ratio<1,SequentialTicksPerSecond>>;
}
