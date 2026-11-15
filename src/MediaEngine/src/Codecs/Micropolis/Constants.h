#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Micropolis::Constants {
inline constexpr std::string_view Id="micropolis.mfm",DisplayName="Micropolis MFM",CompleteKey="MediaEngine.MicropolisRecord",TruncatedKey="MediaEngine.MicropolisTruncated",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid";
inline constexpr std::int32_t SyncZeroCount=3,EncodedByteBitCount=16,SyncBitCount=64,AddressMarkOffset=0,CylinderOffset=1,SectorOffset=2,DataOffset=13,SectorSize=256,ChecksumOffset=269,TrailerOffset=270,TrailerPaddingByteCount=5,RecordByteCount=275,ChecksumDataOffset=1,ChecksumDataLength=268,PreambleByteCount=40,GapBitCount=128,MaximumSectorNumber=255,ChecksumModulus=255,MaximumRecordInputSize=262,ConfidenceSectorWeight=2;
inline constexpr std::uint8_t AddressMark=0xff,LogicalHead=0,SectorSizeCode=1;
inline constexpr double ConfidenceDivisor=24;
inline constexpr std::array<std::uint8_t,8> Sync{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x55,0x55};
}
