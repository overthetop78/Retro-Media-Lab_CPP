#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AmigaMfm::Constants {
inline constexpr std::string_view Id="amiga.mfm",DisplayName="Amiga MFM",SectorCountField="Sectors",SectorNumberField="sectorNumber";
inline constexpr std::string_view StructureKey="MediaEngine.AmigaMfmStructure",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::uint16_t SyncWord=0x4489;
inline constexpr std::array<std::uint8_t,4> SyncBytes{0x44,0x89,0x44,0x89};
inline constexpr std::int32_t SyncWordCount=2,EncodedByteBitCount=16,SyncBitCount=SyncWordCount*EncodedByteBitCount,InfoByteCount=4,FormatByteOffset=0,TrackAndHeadOffset=1,SectorNumberOffset=2,RemainingSectorCountOffset=3,TrackCylinderShift=1;
inline constexpr std::uint8_t FormatByte=0xff,TrackHeadMask=1,NibbleMask=15,SectorSizeCode=2;
inline constexpr std::int32_t LabelByteCount=16,HeaderParitySourceByteCount=InfoByteCount+LabelByteCount,HeaderParityHighOffset=22,HeaderParityLowOffset=23,DataParityHighOffset=26,DataParityLowOffset=27,ParityFieldByteCount=8,EncodedHeaderByteCount=HeaderParitySourceByteCount+ParityFieldByteCount,EncodedDataOffset=EncodedHeaderByteCount,EncodedDataByteCount=512,EncodedSectorByteCount=EncodedHeaderByteCount+EncodedDataByteCount,SectorByteCount=512;
inline constexpr std::int32_t DoubleDensitySectorsPerTrack=11,HighDensitySectorsPerTrack=22,HighDensityCredibleSectorCount=17,HighDensityCredibleTrackCount=2,ConfidenceSectorWeight=3,NibbleBitCount=4,NibbleLastBit=3,OddFirstBit=7,EvenFirstBit=6,BitStride=2,LeadingGapBitCount=100,TrailingGapBitCount=128,MaximumCylinder=127,MaximumHead=1,MaximumSectorNumber=255,MaximumRemainingSectorCount=255;
inline constexpr double ConfidenceDivisor=44;
}
