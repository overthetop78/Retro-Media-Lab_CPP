#pragma once
#include <cstdint>
namespace RetroMediaLab::MediaFileSystems::BinaryConstants {
inline constexpr std::uint64_t BitsPerByte = 8;
inline constexpr std::uint64_t UInt24Bytes = 3;
inline constexpr std::uint64_t CrcPolynomial = 3988292384;
inline constexpr std::uint64_t CrcFieldSize = 4;
inline constexpr std::uint64_t GuidLength = 16;
inline constexpr std::uint64_t GuidFirstWord = 4;
inline constexpr std::uint64_t GuidSecondWord = 6;
inline constexpr std::uint64_t GuidTailOffset = 8;
inline constexpr std::uint64_t GuidTailSize = 8;
}
