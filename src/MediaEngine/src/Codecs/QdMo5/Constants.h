#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::QdMo5::Constants {
inline constexpr std::string_view Id = "qdmo5.mfm", DisplayName = "QD MO5 MFM";
inline constexpr std::string_view PrefixAttribute = "prefix", SectorField = "sector number";
inline constexpr std::string_view HeaderKey = "MediaEngine.QdHeader", DataKey = "MediaEngine.QdData", TruncatedHeaderKey = "MediaEngine.QdTruncatedHeader", TruncatedDataKey = "MediaEngine.QdTruncatedData", UnpairedKey = "MediaEngine.QdUnpaired";
inline constexpr std::string_view ValidKey = "MediaEngine.IntegrityValid", InvalidKey = "MediaEngine.IntegrityInvalid", UnavailableKey = "MediaEngine.IntegrityUnavailable", HexDigits = "0123456789ABCDEF";
inline constexpr std::int32_t PreambleBytes = 10, PreambleBits = 80, MarkBits = 96, EncodedByteBits = 16;
inline constexpr std::int32_t SectorNumberBytes = 2, ReservedBytes = 13, HeaderTailBytes = 15, HeaderBits = 336;
inline constexpr std::int32_t SectorSize = 128, DataBytes = 130, DataBits = 2160, SearchBits = 832;
inline constexpr std::int32_t HeaderGapBits = 160, DataGapBits = 128, MaximumSector = 65535, MaximumPrefix = 255;
inline constexpr std::int32_t BitsPerByte = 8, NibbleBits = 4, NibbleMask = 15, ConfidenceWeight = 2;
inline constexpr std::uint8_t HeaderAddressMark = 0xfb, DefaultPrefix = 0x5a, LogicalCylinder = 0, LogicalHead = 0, SectorSizeCode = 0, PaddingByte = 0;
inline constexpr double ConfidenceDivisor = 20;
inline constexpr std::array<std::uint8_t, 10> Preamble{0xa9,0x14,0xa9,0x14,0xa9,0x14,0xa9,0x14,0xa9,0x14};
inline constexpr std::array<std::uint8_t, 12> HeaderMark{0xa9,0x14,0xa9,0x14,0xa9,0x14,0xa9,0x14,0xa9,0x14,0x44,0x91};
}

