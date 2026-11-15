#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Apridisk::Constants {
inline constexpr std::uint32_t HeaderSize=128,BlockSize=512,Cylinders=70,Heads=1,SectorsPerTrack=9,BlockCount=Cylinders*Heads*SectorsPerTrack,RecordHeaderSize=16,SectorRecord=0xe31d0001;
inline constexpr std::uint16_t Uncompressed=0x9e90,Compressed=0x3e5a;
inline constexpr std::size_t CompressionOffset=4,HeaderLengthOffset=6,PayloadLengthOffset=8,HeadOffset=12,SectorOffset=13,CylinderOffset=14,RunSize=3,RunValueOffset=2,BitsPerByte=8;
inline constexpr std::string_view Signature="ACT Apricot disk image\x1a\x04",Extension=".dsk",FormatId=MediaFileSystems::Formats::ApricotPcXi315;
inline constexpr std::string_view WriterId="apridisk";
}
