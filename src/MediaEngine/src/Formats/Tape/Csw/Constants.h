#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Csw::Constants {
inline constexpr std::string_view FormatId="tape.csw",Extension=".csw",WriterId="tape-csw",Signature="Compressed Square Wave",Application="Retro Media Lab";
inline constexpr std::size_t HeaderV1=32,HeaderV2=52,TerminatorOffset=22,MajorOffset=23,RateOffset=25,CountOffset=29,CompressionV1=27,CompressionV2=33,FlagsV1=28,FlagsV2=34,ExtensionOffset=35,ApplicationOffset=36,ExtendedBytes=4,CompressionBuffer=65536,MaximumSamples=128*1024*1024;
inline constexpr std::uint8_t Terminator=0x1a,Version1=1,Version2=2,Rle=1,Zrle=2,HighFlag=1,MaximumShortPulse=255;
inline constexpr std::int16_t Amplitude=16000;
inline constexpr int ByteBits=8;
}
