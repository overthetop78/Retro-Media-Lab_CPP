#pragma once
#include "../Common/PcmConstants.h"
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::VocTape::Constants {
inline constexpr std::string_view FormatId="tape.voc",WriterId="tape-voc",Extension=".voc",Signature="Creative Voice File\x1a",TimeConstantKey="vocTimeConstant",ChannelKey="channel",InterleavedKey="interleaved";
inline constexpr int HeaderSize=26,DataOffset=20,VersionOffset=22,ChecksumOffset=24,LengthBytes=3,BlockHeaderSize=4,SoundFields=2,SilenceBytes=3,MarkerBytes=2,MaximumByte=255,RateBase=256,RateNumerator=1000000,SampleBias=128,ByteBits=8,SampleBits=16;
inline constexpr std::uint16_t Version=0x0114,ChecksumBase=0x1234;
inline constexpr std::uint32_t MaximumBlock=0x00ffffff,MaximumFirstBlock=0x00fffffd;
inline constexpr std::uint8_t Terminator=0,Sound=1,Continuation=2,Silence=3,Marker=4,Text=5;
}
