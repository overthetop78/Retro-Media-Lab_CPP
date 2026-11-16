#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::MfmEmu::Constants {
inline constexpr std::array<std::uint8_t,8> Signature{0xee,0x4d,0x46,0x4d,0x0d,0x0a,0x1a,0};
inline constexpr std::uint32_t CurrentVersion=0x02020200,TypeMask=0xff000000,FileType=0x02000000,MajorMask=0x00ff0000,FirstMajor=0x00010000,LatestMajor=0x00020000,ReservedVersionMask=0xff,StringVersion=0x02020000,StartTimeVersion=0x02020200,TrackMarker=0x12345678,AdaptecSampleRate=10000000,FirstWordBit=0x80000000;
inline constexpr std::size_t FixedHeaderLength=36,VersionOffset=8,HeaderLengthOffset=12,TrackDataLengthOffset=16,TrackHeaderLengthOffset=20,CylinderCountOffset=24,HeadCountOffset=28,SampleRateOffset=32,WordLength=4,MinimumTrackHeaderLength=12,TrackCylinderOffset=4,TrackHeadOffset=8,StringTerminatorLength=1,CopyBufferLength=65536,CreationExtraWords=3;
inline constexpr std::int32_t EndAddress=-1,AdaptecTrackByteLength=20836,BitsPerByte=8,BitsPerWord=32;
inline constexpr std::uint64_t NanosecondsPerSecond=1000000000,MaximumLength=0x7fffffffffffffffULL,MaximumSignedField=0x7fffffff;
inline constexpr char Null='\0';
inline constexpr std::string_view FormatId="hdd.mfm-emu",WriterId="mfm-emu",Extension=".emu",AdfsId="acorn-adfs";
}
