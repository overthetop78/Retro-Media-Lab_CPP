#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <limits>
namespace RetroMediaLab::MediaEngine::Northstar::Constants {
inline constexpr std::string_view Id="northstar.mfm",DisplayName="NorthStar hard-sectored MFM",HeaderKey="MediaEngine.NorthstarHeader",TruncatedKey="MediaEngine.NorthstarTruncated",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::int32_t EncodedByteBitCount=16,MarkBitCount=128,IdentityBitCount=16,SectorSize=512,PayloadBitCount=8192,FullBlockBitCount=8352,CylinderShift=4,MaximumCylinder=15,MaximumSector=15,GapBitCount=128,ScanAdvance=127,ConfidenceSectorWeight=2,ChecksumByteCount=1,FmSectorSize=256,FmDataByteOffset=2,MfmDataByteOffset=3,FmWindowExtraBytes=3,MfmWindowExtraBytes=4,MaximumAddress=255;
inline constexpr std::uint8_t CylinderMask=15,SectorMask=15,LogicalHead=0,SectorSizeCode=2,FmSectorSizeCode=1,AddressMark=0xfb;
inline constexpr double ConfidenceDivisor=20,Half=0.5,Two=2;
inline constexpr std::int64_t MaximumCellCount=std::numeric_limits<std::int32_t>::max();
inline constexpr std::array<std::uint8_t,8> SyncBytes{0,0,0,0,0,0,0,AddressMark};
inline constexpr std::array<std::uint8_t,2> HardSyncBytes{0,AddressMark};
}
