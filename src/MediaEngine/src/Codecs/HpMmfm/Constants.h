#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::HpMmfm::Constants {
inline constexpr std::string_view Id="hp.mmfm",DisplayName="HP MMFM",HeaderKey="MediaEngine.HpHeader",DataKey="MediaEngine.HpData",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid";
inline constexpr std::int32_t SyncBitCount=32,HeaderByteCount=4,EncodedByteBitCount=16,HeaderBitCount=96,HeaderCylinderOffset=0,HeaderSectorOffset=1,HeadShift=7,MaximumCylinder=255,MaximumHead=1,MaximumSector=127,SectorSize=256,EncodedDataByteCount=258,MinimumDataSearchOffsetBits=128,MaximumDataSearchOffsetBits=928,HeaderGapBitCount=128,DataGapBitCount=256,ConfidenceSectorWeight=2,PairByteCount=2;
inline constexpr std::uint8_t SectorMask=0x7f,SectorSizeCode=1;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,4> SectorSync{0x55,0x55,0x2a,0x54},DataSync{0x55,0x55,0x2a,0x44};
}
