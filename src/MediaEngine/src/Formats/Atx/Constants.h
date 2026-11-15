#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Atx::Constants {
inline constexpr std::uint32_t Signature=0x58385441;
inline constexpr std::size_t FileHeader=48,VersionOffset=4,TrackDataOffset=28,TrackHeader=32,NextOffset=0,TypeOffset=4,NumberOffset=8,CountOffset=10,RelativeDataOffset=20,ListHeader=8,SectorHeader=8,SectorNumberOffset=0,StatusOffset=1,PositionOffset=2,DataOffset=4,WordBytes=2,DwordBytes=4,BitsPerByte=8;
inline constexpr std::uint8_t MissingData=0x10;
inline constexpr std::uint32_t Tracks=40,StandardSectors=18,MaxSectors=26,SectorSize=128,MaxRecords=64;
inline constexpr std::uint64_t MaxOffset=2147483647;
inline constexpr std::string_view TrackDataField="trackData",NextField="nextTrack",HeaderField="trackHeader",ListField="sectorList",DataField="sectorData";
}
