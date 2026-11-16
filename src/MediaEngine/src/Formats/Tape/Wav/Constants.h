#pragma once
#include "../Common/PcmConstants.h"
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::WavTape::Constants {
inline constexpr std::string_view FormatId="tape.wav",WriterId="tape-wav",Extension=".wav",Riff="RIFF",Wave="WAVE",Format="fmt ",Data="data",ChannelKey="channel",InterleavedKey="interleaved",ChunkKey="chunkId";
inline constexpr int HeaderSize=12,ChunkHeaderSize=8,FormatSize=16,TagSize=4,LengthOffset=4,WaveOffset=8,ChannelsOffset=2,RateOffset=4,AlignOffset=12,BitsOffset=14,MaximumChannels=32,MaximumRate=768000,BufferSize=65536;
inline constexpr int OutputHeaderSize=44,OutputFormatOffset=12,OutputFormatLengthOffset=16,OutputTagOffset=20,OutputChannelsOffset=22,OutputRateOffset=24,OutputByteRateOffset=28,OutputAlignOffset=32,OutputBitsOffset=34,OutputDataOffset=36,OutputLengthOffset=40;
inline constexpr std::uint16_t PcmTag=1;
inline constexpr std::uint64_t MaximumRiffLength=0xffffffff;
}
