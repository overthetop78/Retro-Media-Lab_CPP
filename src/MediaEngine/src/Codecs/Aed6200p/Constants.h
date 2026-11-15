#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Aed6200p::Constants {
inline constexpr std::string_view Id = "aed6200p.mfm";
inline constexpr std::string_view DisplayName = "AED 6200P MFM";
inline constexpr std::string_view CylinderField = "cylinder", SectorField = "sector", SizeField = "size";
inline constexpr std::string_view HeaderKey = "MediaEngine.AedHeader", DataKey = "MediaEngine.AedData";
inline constexpr std::string_view TruncatedHeaderKey = "MediaEngine.AedTruncatedHeader";
inline constexpr std::string_view TruncatedDataKey = "MediaEngine.AedTruncatedData";
inline constexpr std::string_view UnpairedKey = "MediaEngine.AedUnpaired";
inline constexpr std::string_view ValidKey = "MediaEngine.IntegrityValid";
inline constexpr std::string_view InvalidKey = "MediaEngine.IntegrityInvalid";
inline constexpr std::string_view UnavailableKey = "MediaEngine.IntegrityUnavailable";
inline constexpr std::string_view HexDigits = "0123456789ABCDEF";
inline constexpr std::int32_t HeaderIdentityByteCount = 5, HeaderByteCount = 7;
inline constexpr std::int32_t HeaderMarkOffset = 0, CylinderOffset = 1, SizeLowOffset = 2;
inline constexpr std::int32_t SectorOffset = 3, SizeHighOffset = 4;
inline constexpr std::int32_t BitsPerByte = 8, EncodedByteBitCount = 16;
inline constexpr std::int32_t HeaderBitCount = HeaderByteCount * EncodedByteBitCount;
inline constexpr std::int32_t SearchWindowBitCount = 104 * BitsPerByte;
inline constexpr std::int32_t DataMarkByteCount = 1, CrcByteCount = 2;
inline constexpr std::int32_t FirstGapBitCount = 64, SecondGapBitCount = 128;
inline constexpr std::int32_t MaximumAddress = 255, MaximumSize = 65535, LogicalHead = 0;
inline constexpr std::int32_t NibbleBits = 4, NibbleMask = 15;
inline constexpr std::uint8_t HeaderMark = 0xc6, FirstDataMark = 0xc0, LastDataMark = 0xc3;
inline constexpr std::size_t PatternByteCount = 2;
inline constexpr std::array<std::uint8_t, PatternByteCount> HeaderPattern{0x50, 0x94};
struct DataMarkDefinition {
    std::uint8_t mark;
    std::array<std::uint8_t, PatternByteCount> pattern;
    bool deleted;
};
inline constexpr DataMarkDefinition DeletedDefinition{0xc0, {0x50, 0x8a}, true};
inline constexpr DataMarkDefinition AlternateC1Definition{0xc1, {0x50, 0x89}, false};
inline constexpr DataMarkDefinition AlternateC2Definition{0xc2, {0x50, 0x84}, false};
inline constexpr DataMarkDefinition NormalDefinition{0xc3, {0x50, 0x85}, false};
inline constexpr std::array DataMarks{DeletedDefinition, AlternateC1Definition, AlternateC2Definition, NormalDefinition};
inline constexpr std::size_t NormalDataMarkIndex = 3, DeletedDataMarkIndex = 0;
inline constexpr std::int32_t MarkBitCount = std::int32_t(HeaderPattern.size()) * BitsPerByte;
}

