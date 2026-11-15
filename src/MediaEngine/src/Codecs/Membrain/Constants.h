#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Membrain::Constants {
inline constexpr std::string_view Id = "membrain.mfm", DisplayName = "Membrain MFM";
inline constexpr std::string_view HeaderKey = "MediaEngine.MembrainHeader", DataKey = "MediaEngine.MembrainData";
inline constexpr std::string_view TruncatedHeaderKey = "MediaEngine.MembrainTruncatedHeader";
inline constexpr std::string_view TruncatedDataKey = "MediaEngine.MembrainTruncatedData";
inline constexpr std::string_view UnpairedKey = "MediaEngine.MembrainUnpaired";
inline constexpr std::string_view ValidKey = "MediaEngine.IntegrityValid", InvalidKey = "MediaEngine.IntegrityInvalid", UnavailableKey = "MediaEngine.IntegrityUnavailable";
inline constexpr std::string_view HexDigits = "0123456789ABCDEF";
inline constexpr std::int32_t EncodedByteBits = 16, PatternBits = 32;
inline constexpr std::size_t AddressByteCount = 2;
inline constexpr std::int32_t HeaderBytes = 6, HeaderMarkOffset = 1, CylinderHighOffset = 2, PackedOffset = 3;
inline constexpr std::int32_t HeaderBits = HeaderBytes * EncodedByteBits;
inline constexpr std::int32_t DataPrefixBytes = 2, CrcBytes = 2, SectorSize = 512;
inline constexpr std::int32_t DataBytes = DataPrefixBytes + SectorSize + CrcBytes;
inline constexpr std::int32_t CylinderLowBits = 3, CylinderLowShift = 5, HeadShift = 4;
inline constexpr std::int32_t MaximumCylinder = 255, MaximumHead = 1, MaximumSector = 15;
inline constexpr std::int32_t SearchStart = 1, SearchBits = 832, HeaderGapBits = 64, DataGapBits = 128;
inline constexpr std::int32_t BitsPerByte = 8, NibbleBits = 4, NibbleMask = 15, ConfidenceWeight = 2;
inline constexpr std::uint8_t SyncByte = 0xa1, HeaderMark = 0xfe, FirstDataMark = 0xf8, LastDataMark = 0xfb, SectorSizeCode = 2;
inline constexpr std::uint8_t CylinderHighMask = 0x1f, CylinderLowValueMask = 0x07, CylinderLowMask = 0xe0, HeadMask = 1, SectorMask = 0x0f;
inline constexpr std::uint16_t EncodedSync = 0x4489;
inline constexpr double ConfidenceDivisor = 20;
inline constexpr std::array<std::uint8_t, 4> HeaderPattern{0x44, 0x89, 0x55, 0x54};
inline constexpr std::array<std::uint8_t, 4> DataPattern{0x44, 0x89, 0x55, 0x4a};
}

