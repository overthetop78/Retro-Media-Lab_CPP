#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Scp::DiskTypes {
inline constexpr std::uint8_t Commodore64=0x00, Amiga=0x04, AmigaHighDensity=0x08;
inline constexpr std::uint8_t Atari8BitSingleDensity=0x10, Atari8BitDoubleDensity=0x11, Atari8BitEnhancedDensity=0x12;
inline constexpr std::uint8_t AtariStSingleSided=0x14, AtariStDoubleSided=0x15;
inline constexpr std::uint8_t AppleII=0x20, AppleIIProDos=0x21, AppleMacintosh400=0x24, AppleMacintosh800=0x25, AppleMacintosh1440=0x26;
inline constexpr std::uint8_t IbmPc360=0x30, IbmPc720=0x31, IbmPc1200=0x32, IbmPc1440=0x33, AmstradCpc=0x70;
inline constexpr std::uint8_t Other320=0x80, Other1200=0x81, Other720=0x84, Other1440=0x85;
inline constexpr std::uint64_t BytesPerKibibyte=1024, Capacity720=720, Capacity800=800, Capacity880=880, Capacity1200=1200, Capacity1440=1440;
inline constexpr std::uint32_t SingleHead=1;
inline constexpr std::string_view ProDosKeyword="prodos";
}

