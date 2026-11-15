#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Tycom::Constants {
inline constexpr std::string_view Id = "tycom.fm", DisplayName = "TYCOM FM";
inline constexpr std::string_view CylinderField = "cylinder", SectorField = "sector number";
inline constexpr std::string_view HeaderKey = "MediaEngine.TycomHeader", HeaderNoMarkKey = "MediaEngine.TycomHeaderNoMark", InvalidHeaderKey = "MediaEngine.TycomInvalidHeader", DataKey = "MediaEngine.TycomData";
inline constexpr std::string_view TruncatedHeaderKey = "MediaEngine.TycomTruncatedHeader", TruncatedDataKey = "MediaEngine.TycomTruncatedData", UnpairedKey = "MediaEngine.TycomUnpaired";
inline constexpr std::string_view ValidKey = "MediaEngine.IntegrityValid", InvalidKey = "MediaEngine.IntegrityInvalid", UnavailableKey = "MediaEngine.IntegrityUnavailable", HexDigits = "0123456789ABCDEF";
inline constexpr std::uint8_t HeaderMark = 0xfe, FirstDataMark = 0xf8, LastDataMark = 0xfb, LogicalHead = 0, SizeCode = 0;
inline constexpr std::int32_t MarkBits = 32, HeaderBytes = 4, EncodedByteBits = 32, HeaderBits = 160;
inline constexpr std::int32_t SectorSize = 128, DataBytes = 131, DataBits = DataBytes * EncodedByteBits;
inline constexpr std::int32_t GapBits = 64, SearchBits = 1664, MaximumAddress = 255;
inline constexpr std::int32_t BitsPerByte = 8, NibbleBits = 4, NibbleMask = 15, ConfidenceWeight = 2;
inline constexpr double ConfidenceDivisor = 20;
}

