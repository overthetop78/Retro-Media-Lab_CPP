#pragma once
#include <RetroMediaLab/MediaEngine/TrackEncodingAttributeKeys.h>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::IsoFm::Constants {
inline constexpr std::string_view Id="iso.fm",DisplayName="ISO FM (simple densité)",PhysicalAttribute=TrackEncodingAttributeKeys::FmPhysicalLayout,CompleteAttribute=TrackEncodingAttributeKeys::RequireCompletePhysicalTrack,RecordedCylinderField="RecordedCylinder",RecordedHeadField="RecordedHead";
inline constexpr std::int32_t PhysicalEnabled=1,InitialGapBytes=16,SyncBytes=6,InterFieldGapBytes=11,InterSectorGapBytes=11,EncodedByteBits=16,CrcBytes=2,HeaderGapBits=160,DataGapBits=256,MaximumAddress=255,MaximumSizeCode=7;
inline constexpr std::uint8_t GapFill=0xff,SyncFill=0,IdMark=0xfe,DataMark=0xfb,DeletedMark=0xf8;
inline constexpr std::int32_t MaximumPeriod=2147483647,CellsPerDataBit=2,HeaderFields=4,HeaderBytesAfterMark=6,HeaderBits=112,ConfidenceWeight=2,MaximumSectorBytes=16384;
inline constexpr std::int32_t HeaderCylinder=0,HeaderHead=1,HeaderSector=2,HeaderCode=3,MaximumDataBits=(MaximumSectorBytes+CrcBytes)*EncodedByteBits;
inline constexpr double ConfidenceDivisor=18;
inline constexpr std::string_view HeaderKey="MediaEngine.FmHeader",DataKey="MediaEngine.FmData",DeletedKey="MediaEngine.FmDeleted",UnpairedKey="MediaEngine.FmUnpaired",UnpairedDeletedKey="MediaEngine.FmUnpairedDeleted",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::uint16_t IdPattern=0xf57e,DataPattern=0xf56f,DeletedPattern=0xf56a;
}
