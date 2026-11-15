#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Heathkit::Constants {
inline constexpr std::string_view Id="heathkit.fm",DisplayName="Heathkit hard-sectored FM",HeaderKey="MediaEngine.HeathkitHeader",DataKey="MediaEngine.HeathkitData",TruncatedKey="MediaEngine.HeathkitTruncated",UnpairedKey="MediaEngine.HeathkitUnpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable",VolumeAttributeName="volume";
inline constexpr std::int32_t MarkBitCount=64,HeaderByteCount=4,HeaderVolumeOffset=0,HeaderCylinderOffset=1,HeaderSectorOffset=2,EncodedByteBitCount=16,HeaderBitCount=128,SectorSize=256,DataBlockByteCount=257,MaximumDataSearchDistanceBits=832,HeaderGapBitCount=160,DataGapBitCount=128,ConfidenceSectorWeight=2,DefaultVolume=0,MaximumAddress=255,ChecksumByteCount=1;
inline constexpr std::uint8_t LogicalHead=0,SectorSizeCode=1,AddressMark=0xbf;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,4> SyncBytes{0,0,0,AddressMark};
}
