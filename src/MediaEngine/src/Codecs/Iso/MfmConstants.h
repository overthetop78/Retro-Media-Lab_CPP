#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::IsoMfm::Constants {
inline constexpr std::string_view Id="iso.mfm",DisplayName="ISO MFM (Atari ST / IBM PC)",PhysicalAttribute="mfmPhysicalLayout",CompleteAttribute="requireCompletePhysicalTrack",RecordedCylinderField="RecordedCylinder",RecordedHeadField="RecordedHead";
inline constexpr std::int32_t PhysicalEnabled=1,InitialGapBytes=80,SyncZeroBytes=12,InterFieldGapBytes=22,InterSectorGapBytes=24,SyncByteCount=3,EncodedByteBits=16,CrcBytes=2,HeaderGapBits=160,DataGapBits=256,MaximumAddress=255,MaximumSizeCode=7;
inline constexpr std::uint8_t GapFill=0x4e,SyncFill=0,SyncByte=0xa1,IdMark=0xfe,DataMark=0xfb,DeletedMark=0xf8;
inline constexpr std::uint32_t SyncBits=48,SyncAndMarkBits=64,HeaderFields=4,HeaderBytesAfterMark=6,HeaderBits=160,CircularTailBits=20000,ConfidenceWeight=2;
inline constexpr std::uint32_t HeaderCylinder=0,HeaderHead=1,HeaderSector=2,HeaderCode=3;
inline constexpr std::uint16_t SyncPattern=0x4489;
inline constexpr std::uint32_t ValidScoreWeight=1000,DataScoreWeight=10,SectorScoreWeight=1,HexMask=15,HexShift=4;
inline constexpr double ConfidenceDivisor=12;
inline constexpr std::array<double,7> PllFactors{1.0,0.98,1.02,0.96,1.04,0.94,1.06};
inline constexpr std::string_view HexDigits="0123456789ABCDEF",HeaderKey="MediaEngine.MfmHeader",DataKey="MediaEngine.MfmData",DeletedKey="MediaEngine.MfmDeleted",UnknownKey="MediaEngine.MfmUnknown",UnpairedKey="MediaEngine.MfmUnpaired",UnpairedDeletedKey="MediaEngine.MfmUnpairedDeleted";
inline constexpr std::string_view ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::array<std::uint8_t,6> EncodedSync{0x44,0x89,0x44,0x89,0x44,0x89};
}
