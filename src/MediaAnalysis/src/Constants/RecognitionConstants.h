#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaAnalysis::RecognitionConstants {
inline constexpr std::string_view Empty = "";
inline constexpr auto ExtensionPrefix = '.';
inline constexpr char32_t DottedCapitalI = 0x0130;
inline constexpr std::size_t RuleCount = 531;
inline constexpr std::size_t TextSampleLimit = 512;
inline constexpr std::size_t AtariStandardCartridge16K = 16 * 1024;
inline constexpr std::uint8_t Tab = 9, LineFeed = 10, CarriageReturn = 13, PrintableFirst = 32, PrintableEnd = 127, AtasciiEol = 0x9B, AtasciiMask = 0x7F;
inline constexpr std::size_t PrintableNumerator = 9, PrintableDenominator = 10;
inline constexpr char UpperFirst = 'A', UpperLast = 'Z', CaseDelta = 'a' - 'A';
}
