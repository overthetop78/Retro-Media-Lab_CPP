#pragma once
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Primitives::Crc32CConstants {
inline constexpr std::uint32_t Polynomial=0x82f63b78,Initial=0xffffffff,ByteMask=0xff;
inline constexpr std::size_t TableSize=256;
inline constexpr int ByteBits=8;
}
