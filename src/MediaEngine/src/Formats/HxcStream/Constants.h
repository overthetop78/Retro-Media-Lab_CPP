#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::HxcStream::Constants {
inline constexpr std::uint32_t ChunkSignature=0x484b4843,MetadataBlock=0,PackedIoBlock=1,PackedStreamBlock=2,CrcSeed=0xffffffff;
inline constexpr std::size_t ChunkHeaderBytes=12,CrcBytes=4,BlockHeaderBytes=8,IoExtraBytes=8,StreamExtraBytes=12,FieldBytes=4,WordBytes=2,ByteBits=8,Alignment=4;
inline constexpr std::size_t MaximumChunkBytes=64*1024*1024,MaximumUnpackedBytes=64*1024*1024,MaximumSignedSize=2147483647,CrcChunkBytes=65536;
inline constexpr std::uint32_t MaximumPulse=0x0fffffff,IoSampleTicks=16,TrailingIoSamples=2,NanosecondsPerSecond=1000000000;
inline constexpr std::uint64_t MaximumTime=9223372036854775807ULL;
inline constexpr std::array<std::int32_t,2> Resolutions{40,20};
inline constexpr std::uint8_t IndexFlag=1,WriteProtectedFlag=1<<5,TrueByte=1,FalseByte=0;
inline constexpr std::uint32_t PulseOneLimit=0x80,PulseTwoLimit=0x4000,PulseThreeLimit=0x200000,PulseTwoPrefix=0x80,PulseThreePrefix=0xc0,PulseFourPrefix=0xe0;
inline constexpr std::size_t ChunkSizeOffset=4;
inline constexpr std::size_t TwoByteShift=16,ThreeByteShift=24;
inline constexpr std::string_view NativePrefix="hxcstream.native.",DigestPrefix="hxcstream.digest.",FieldsPrefix="hxcstream.field.",SampleRateKey="sample_rate_hz";
inline constexpr char Separator='.',Space=' ',Newline='\n',CarriageReturn='\r',Null='\0';
inline constexpr std::uint32_t MaximumPulseCount=500000000,MaximumTrack=83,MaximumHead=1,MaximumRevolutionTicks=4294967295;
inline constexpr std::int32_t DefaultResolution=40;
inline constexpr std::size_t IoHeaderBytes=16,StreamHeaderBytes=20,PackedSizeOffset=8,UnpackedSizeOffset=12,PulseCountOffset=16;
inline constexpr std::uint8_t PulseTwoMask=0xc0,PulseThreeMask=0xe0,PulseFourMask=0xf0,PulseTwoData=0x3f,PulseThreeData=0x1f,PulseFourData=0x0f;
inline constexpr std::size_t TwoTailBytes=1,ThreeTailBytes=2,FourTailBytes=3;
inline constexpr std::array<std::uint8_t,4> Signature{'C','H','K','H'};
inline constexpr std::string_view FormatId="raw.hxcstream",Extension=".hxcstream",FilePattern="^(.*?)(\\d{2})\\.([01])\\.hxcstream$";
inline constexpr int PrefixGroup=1,CylinderGroup=2,HeadGroup=3;
inline constexpr std::string_view LineSeparators="\r\n";
inline constexpr std::int32_t LowSampleRate=25000000,HighSampleRate=50000000;
inline constexpr std::size_t MinimumIndexEdges=2;
}
