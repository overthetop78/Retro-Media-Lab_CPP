#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Commodore900Gcr::Constants {
inline constexpr std::string_view Id="commodore900.gcr",DisplayName="Commodore 900 GCR";
inline constexpr std::uint8_t HeaderMark=0x08,DataMark=0x07,LogicalHead=0,SizeCode=2;
inline constexpr std::uint32_t HeaderBytes=4,HeaderCylinder=1,HeaderSector=2,SectorBytes=512,DataBytes=514,PayloadOffset=1;
inline constexpr std::uint32_t EncodedByteBits=10,MinimumSync=10,SyncGap=40,RecordGap=120,ExpectedSectors=13,MaximumAddress=255;
inline constexpr std::uint64_t MaximumBits=2147483647;
inline constexpr std::string_view SyncKey="MediaEngine.C900GcrSync",HeaderKey="MediaEngine.C900GcrHeader",DataKey="MediaEngine.C900GcrData",UnpairedKey="MediaEngine.C900GcrUnpaired";
inline constexpr std::string_view ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
}
