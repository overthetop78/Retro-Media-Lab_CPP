#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::DataGeneral::Constants {
inline constexpr std::string_view Id="datageneral.fm",DisplayName="Data General 2F",HeaderKey="MediaEngine.DataGeneralHeader",DataKey="MediaEngine.DataGeneralData",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid";
inline constexpr std::int32_t EncodedByteBitCount=16,EncodedSyncBitCount=32,IdentityByteCount=2,CylinderAndHeadOffset=0,SectorOffset=1,MaximumCylinder=127,MaximumHead=1,HeadShift=7,SectorShift=2,MaximumSectorNumber=7,MinimumSyncDistance=32,MaximumSyncDistance=256,SectorSize=512,DataBlockByteCount=514,ChecksumHighOffset=512,ChecksumLowOffset=513,HeaderGapBitCount=64,DataGapBitCount=128,ConfidenceSectorWeight=2,BitsPerByte=8;
inline constexpr std::uint8_t CylinderMask=0x7f,HeadMask=0x80,SectorSizeCode=2,ByteMask=0xff,TerminalByte=0;
inline constexpr double ConfidenceDivisor=24;
inline constexpr std::array<std::uint8_t,2> SyncBytes{0,1};
}
