#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::BinCue::WaveConstants {
inline constexpr int RiffHeaderBytes=12,ChunkHeaderBytes=8,FormatBytes=16,TagBytes=4,FormOffset=8,ChunkLengthOffset=4,ChannelsOffset=2,RateOffset=4,ByteRateOffset=8,AlignOffset=12,BitsOffset=14,SampleRate=44100,ByteRate=176400,AudioSectorBytes=2352;
inline constexpr std::uint16_t Pcm=1,Stereo=2,SampleBits=16,BlockAlign=4;
inline constexpr std::string_view Riff="RIFF",Wave="WAVE",Format="fmt ",Data="data";
}
