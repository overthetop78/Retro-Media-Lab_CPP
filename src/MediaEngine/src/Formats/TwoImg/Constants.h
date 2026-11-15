#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaEngine::TwoImg::Constants {
inline constexpr std::size_t MinimumHeaderSize=64,SignatureOffset=0,SignatureLength=4,CreatorOffset=4,CreatorLength=4;
inline constexpr std::size_t HeaderSizeOffset=8,VersionOffset=10,ImageFormatOffset=12,FlagsOffset=16,BlockCountOffset=20,DataOffsetOffset=24,DataLengthOffset=28,MinimumDataLength=1;
inline constexpr std::uint16_t SupportedVersion=1;
inline constexpr std::uint32_t ProDosBlockBytes=512;
inline constexpr std::array<std::uint8_t,4> Signature{'2','I','M','G'},Creator{'G','W','G','U'};
inline constexpr std::string_view DosExtension=".do",ProDosExtension=".po",ContainerExtension=".2mg",TargetSeparator=" : ";
inline constexpr auto MaximumSignedField=std::uint32_t(std::numeric_limits<std::int32_t>::max());
}
