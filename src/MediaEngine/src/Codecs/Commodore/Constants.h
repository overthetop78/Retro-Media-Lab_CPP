#pragma once
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::CommodoreGcr::Constants {
inline constexpr std::array<std::uint8_t,16> Encoding={0x0a,0x0b,0x12,0x13,0x0e,0x0f,0x16,0x17,0x09,0x19,0x1a,0x1b,0x0d,0x1d,0x1e,0x15};
inline constexpr int NibbleBits=5,ByteBits=10,NibbleMask=0x0f,NibbleShift=4;
inline constexpr std::uint64_t MaximumBits=2147483647;
}
