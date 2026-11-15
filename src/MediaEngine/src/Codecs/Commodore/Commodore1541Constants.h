#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Commodore1541Gcr::Constants {
inline constexpr std::string_view Id="commodore.gcr",DisplayName="Commodore GCR",Id2Attribute="id2",Id1Attribute="id1",TrackAttribute="track",TracksPerSideAttribute="tracksPerSide";
inline constexpr std::uint8_t HeaderMark=0x08,DataMark=0x07,LogicalHead=0,SizeCode=1,DefaultId2=0xa1,DefaultId1=0x1a;
inline constexpr std::uint32_t HeaderBytes=6,HeaderChecksumOffset=1,HeaderSector=2,HeaderTrack=3,HeaderId2=4,HeaderId1=5,SectorBytes=256,DataBytes=258,PayloadOffset=1;
inline constexpr std::uint32_t EncodedByteBits=10,MinimumSync=10,LeadingGap=100,RawGap=3,SyncGap=20,HeaderDataGap=6,TrailingGap=32;
inline constexpr std::int32_t TracksPerSide=35,MinimumDiskTrack=1,MaximumDiskTrack=80,MaximumCylinder=39,MaximumByte=255,ConfidenceWeight=2;
inline constexpr double ConfidenceDivisor=42;
inline constexpr std::uint64_t MaximumBits=2147483647;
inline constexpr std::string_view SyncKey="MediaEngine.CbmGcrSync",HeaderKey="MediaEngine.CbmGcrHeader",DataKey="MediaEngine.CbmGcrData";
inline constexpr std::string_view ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
}
