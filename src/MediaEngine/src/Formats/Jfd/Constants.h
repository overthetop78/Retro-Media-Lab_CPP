#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Jfd::Constants {
inline constexpr std::size_t MaximumLength=64*1024*1024,MinimumHeader=304,FieldBytes=4,DescriptorBytes=8,CompressionChunk=16384;
inline constexpr std::size_t LengthOffset=8,TracksOffset=24,SectorsOffset=28,DataOffset=32,DeltaTracksOffset=36,DeltaSectorsOffset=40,DeltaDataOffset=44;
inline constexpr std::array<std::uint8_t,4> Signature{'J','F','D','I'};
inline constexpr std::array<std::uint8_t,2> GzipSignature{0x1f,0x8b};
inline constexpr std::uint32_t Absent=0xffffffff,RawTrack=0x00100000,HeaderError=0x10,DataError=0x20,ErrorMask=HeaderError|DataError;
inline constexpr std::uint32_t MaximumTracks=512,MaximumRecords=256,MaximumSizeCode=7,BaseSectorBytes=128,DefaultSectorBytes=1024;
inline constexpr std::uint32_t SizeMask=15,ByteMask=255,DensityShift=16,NumberShift=8,PositionShift=24,Heads=2,NativeTag=1;
inline constexpr std::array<std::uint32_t,4> Densities{1,2,4,8};
inline constexpr std::int32_t MissingOrder=-1;
inline constexpr int DeflateWindowBits=15,GzipWrapperFlag=16,GzipWindowBits=DeflateWindowBits+GzipWrapperFlag;
inline constexpr std::string_view FormatId="acorn.jfd",NativeKey="jfd.native",GzipKey="jfd.gzip";
}
