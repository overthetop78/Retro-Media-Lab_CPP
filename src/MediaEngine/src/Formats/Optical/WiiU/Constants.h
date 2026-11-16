#pragma once
#include <string_view>
#include <cstdint>
#include <array>
namespace RetroMediaLab::MediaEngine::WiiU::Constants {
inline constexpr std::string_view FormatId="nintendo.wiiu",WriterId="nintendo-wii-u",WudExtension=".wud",WuxExtension=".wux",SystemIdKey="nintendo.wiiu",MediaRole="optical-image",VariantKey="nintendo.wiiu.variant",SectorSizeKey="nintendo.wiiu.sectorSize",LengthKey="nintendo.wiiu.uncompressedSize",IndexCountKey="nintendo.wiiu.indexEntryCount",StoredCountKey="nintendo.wiiu.storedSectorCount",VolumeName="Nintendo Wii U";
inline constexpr int HeaderBytes=28,OpticalBytes=2048,DefaultChunkBytes=0x8000,MinimumChunkBytes=0x100,MaximumChunkBytes=0x10000000,Magic1Offset=4,ChunkOffset=8,LengthOffset=12,IndexBytes=4,IndexBufferBytes=65536,TransferBytes=256*1024;
inline constexpr std::uint32_t Magic0=0x30585557,Magic1=0x1099d02e;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff,MaximumIndexCount=0x7fffffff;
inline constexpr std::array<std::uint8_t,8> Signature{'W','U','X','0',0x2e,0xd0,0x99,0x10};
inline constexpr char ExtensionSeparator='.';
}
