#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Petscii::Constants {
inline constexpr std::uint8_t Terminator=0,Padding=0xa0,Mask=0x7f,DirectStart=0x20,DirectEnd=0x5f,LowerStart=0x60,LowerEnd=0x7a,CaseOffset=0x20;
inline constexpr char16_t AsciiLowerStart=0x61,AsciiLowerEnd=0x7a,LongS=0x17f,UpperS=0x53;
inline constexpr std::string_view Replacement="\xef\xbf\xbd";
}
