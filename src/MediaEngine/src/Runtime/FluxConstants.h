#pragma once
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::RuntimeDetails::FluxConstants {
inline constexpr std::uint32_t Magic=0x31465747,MaximumRevolutions=255,MaximumIntervals=16777216,NanosecondsPerSecond=1000000000;
inline constexpr std::size_t HeaderSize=8,RevolutionHeaderSize=8,UInt32Size=4,ByteBits=8;
inline constexpr std::uint32_t ByteMask=255;
inline constexpr std::size_t MaximumPayload=2147483647;
}
