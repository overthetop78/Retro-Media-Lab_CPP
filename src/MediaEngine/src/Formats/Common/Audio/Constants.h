#pragma once
#include <string_view>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::AudioPcm::Constants {
inline constexpr std::string_view Format="tape.audio",OggExtension=".ogg",ChannelKey="channel",InterleavedKey="interleaved",ExtensionKey="sourceExtension",TrueValue="True";
inline constexpr std::array<std::string_view,6> Extensions{".mp3",".ogg",".flac",".aac",".m4a",".wma"};
inline constexpr int SampleBytes=2,SampleBits=16,MaximumAlign=65535,TicksPerSecond=10000000,UInt8Bias=128;
inline constexpr double PcmMaximum=32767,PcmMinimum=-32768,Int16Scale=32768,Int32Scale=2147483648.0,UInt8Scale=128;
inline constexpr std::uint64_t MaximumBuffer=0x7fffffff;
}
