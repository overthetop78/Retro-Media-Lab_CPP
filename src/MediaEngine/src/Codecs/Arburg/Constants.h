#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Arburg::Constants {
inline constexpr std::string_view Id = "arburg", DisplayName = "Arburg system/data";
inline constexpr std::string_view SystemAttribute = "system";
inline constexpr std::string_view DataKey = "MediaEngine.ArburgData", SystemKey = "MediaEngine.ArburgSystem";
inline constexpr std::string_view ValidKey = "MediaEngine.IntegrityValid", InvalidKey = "MediaEngine.IntegrityInvalid", UnavailableKey = "MediaEngine.IntegrityUnavailable";
inline constexpr std::int32_t DataSize = 0xa00, DataUsefulSize = 0x9fe;
inline constexpr std::int32_t SystemSize = 0xf00, SystemUsefulSize = 0xefe;
inline constexpr std::int32_t ChecksumBytes = 2, BitsPerByte = 8, FmByteBits = 32;
inline constexpr std::int32_t ZeroBits = 2, OneBits = 3, GapBits = 64, MarkBits = 64;
inline constexpr std::int32_t OneThirdBitOffset = 2;
inline constexpr std::int32_t LogicalCylinder = 0, LogicalHead = 0, LogicalSector = 1, SizeCode = 0;
inline constexpr std::int32_t ConfidenceWeight = 2;
inline constexpr double ConfidenceDivisor = 8;
inline constexpr std::int64_t MaximumBytes = 2147483647;
inline constexpr std::int64_t MaximumSystemBytes = MaximumBytes / BitsPerByte / ZeroBits;
inline constexpr std::uint8_t PrefixBit = 0, ZeroSecondBit = 1, OneSecondBit = 0, OneThirdBit = 1, PaddingByte = 0;
inline constexpr std::array<std::uint8_t, 8> DataMark{0x44, 0x44, 0x44, 0x44, 0x55, 0x55, 0x55, 0x55};
inline constexpr std::array<std::uint8_t, 8> SystemMark{0x55, 0x55, 0x55, 0x55, 0x55, 0x24, 0x92, 0x49};
}

