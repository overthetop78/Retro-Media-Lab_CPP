#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Victor9000::Constants {
inline constexpr std::string_view Id="victor9k.gcr",DisplayName="Victor 9000 GCR",SectorField="sector number",HeaderKey="MediaEngine.VictorHeader",DataKey="MediaEngine.VictorData",TruncatedKey="MediaEngine.VictorTruncated",UnpairedKey="MediaEngine.VictorUnpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable",HexDigits="0123456789ABCDEF";
inline constexpr std::int32_t MarkBitCount=64,EncodedDataStartBitOffset=49,EncodedCellStride=2,HeaderByteCount=6,HeaderTypeOffset=0,HeaderCylinderOffset=1,HeaderSectorOffset=2,HeaderSumOffset=3,HeaderId2Offset=4,HeaderId1Offset=5,SectorByteCount=512,DataPrefixOffset=0,DataOffset=1,ChecksumLowOffset=513,ChecksumHighOffset=514,DecodedDataByteCount=515,MaximumCylinder=255,MaximumSector=255,MaximumDataSearchDistanceBits=1568,HeaderGapBitCount=20,DataGapBitCount=64,ConfidenceSectorWeight=2,BitsPerByte=8,HexShift=4,HexMask=15;
inline constexpr std::uint8_t HeaderType=0x06,HeaderId2=0xa1,HeaderId1=0x1a,DataPrefix=0,LogicalHead=0,SectorSizeCode=2;
inline constexpr double ConfidenceDivisor=24;
inline constexpr std::array<std::uint8_t,8> HeaderMark{0x55,0x55,0x55,0x55,0x55,0x55,0x11,0x11},DataMark{0x55,0x55,0x55,0x55,0x55,0x55,0x11,0x04};
}
