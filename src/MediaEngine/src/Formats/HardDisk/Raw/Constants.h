#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::HardDiskRaw::Constants {
inline constexpr std::string_view FormatId="hdd.raw",WriterId="hard-disk-raw",LogicalBlockSizeKey="logicalBlockSize",GptSignature="EFI PART";
inline constexpr std::array<std::string_view,5> Extensions{".raw",".img",".hdd",".hdf",".bin"};
inline constexpr std::int32_t LegacyBlockSize=512,LargeBlockSize=4096,MbrSignatureOffset=510;
inline constexpr std::array<std::uint8_t,2> MbrSignature{0x55,0xaa};
}
