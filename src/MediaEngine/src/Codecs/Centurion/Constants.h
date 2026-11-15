#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Centurion::Constants {
inline constexpr std::string_view Id="centurion.mfm",DisplayName="Centurion MFM",BlockCountField="blockCount",HeaderKey="MediaEngine.CenturionHeader",DataKey="MediaEngine.CenturionData",TruncatedHeaderKey="MediaEngine.CenturionTruncatedHeader",TruncatedDataKey="MediaEngine.CenturionTruncatedData",UnsupportedKey="MediaEngine.CenturionUnsupported",UnpairedKey="MediaEngine.CenturionUnpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::int32_t HeaderByteCount=4,HeaderCylinderOffset=0,HeaderSectorOffset=1,DataPrefixByteCount=3,DataKeyOffset=0,DataSizeOffset=1,DataCrcPrefixByteCount=2,CrcByteCount=2,AllocationBlockSize=256,MinimumBlockCount=1,MaximumAddress=255,MaximumBlockCount=255,SearchDistance=400,HeaderGapBitCount=400,DataGapBitCount=128,EncodedByteBitCount=16,ConfidenceSectorWeight=2,SectorMarkBitCount=32,DataMarkBitCount=32,HeaderBitCount=96,DataPrefixBitCount=80,SectorAdvance=31,DataAdvance=31,BitsPerByte=8;
inline constexpr std::uint8_t SupportedDataKey=0,ReservedPrefix=0,PaddingByte=0,LogicalHead=0;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,4> SectorMark{0x91,0x22,0x44,0x89},DataMark{0xaa,0xaa,0xaa,0xa9};
}
