#pragma once
#include <string_view>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::TapePcm::Constants {
inline constexpr std::string_view RawFormat="tape.raw",RawWriter="tape-raw",RawExtension=".raw",RateKey="sampleRate",ChannelsKey="channels",BitsKey="bitsPerSample",AlignKey="blockAlign",SignedKey="signed",EndianKey="littleEndian",TrueValue="true",FalseValue="false";
inline constexpr int SilenceDivisor=1000,ProfileNumberFields=3,TripleBits=24,ThirdByte=2,EvenDivisor=2,DefaultRate=48000,DefaultChannels=1,DefaultBits=16,MinimumRate=8000,MaximumRate=192000,MaximumRawChannels=2,ByteBits=8,WordBits=32,Signed8Bias=128,TicksPerSecond=10000000,MinimumSignalBits=16;
inline constexpr std::int64_t MaximumBuffer=0x7fffffff;
inline constexpr std::array<int,4> SupportedBits{8,16,24,32};
inline constexpr std::uint32_t Sign24=0x00800000,Extend24=0xff000000;
inline constexpr double Half=0.5;
inline constexpr char Plus='+';
}
