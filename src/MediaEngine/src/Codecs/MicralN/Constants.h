#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::MicralN::Constants {
inline constexpr std::string_view Id="micraln.fm",DisplayName="Micral N hard-sectored FM",CompleteKey="MediaEngine.MicralBlock",TruncatedKey="MediaEngine.MicralTruncated",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid";
inline constexpr std::int32_t SectorSize=128,SectorOffset=0,CylinderOffset=1,DataOffset=2,ChecksumOffset=130,BytesAfterMark=131,BlockBitCount=2160,MarkBitCount=64,EncodedByteBitCount=16,GapBitCount=128,ScanAdvance=63,MaximumSectorNumber=255,ConfidenceSectorWeight=2;
inline constexpr std::uint8_t InitialChecksum=0,ComplementMask=0xff,CarryMask=0x80,LogicalHead=0,SectorSizeCode=0;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,8> SectorMark{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xff,0xff};
}
