#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Cp2::Constants {
inline constexpr std::string_view Signature="SOFTWARE PIRATES",Extension=".cp2";
inline constexpr std::size_t MinimumFileLength=34,SignatureOffset=0,FirstGroupOffset=28,GroupHeaderSize=4,MetadataLengthOffset=2,LengthFieldSize=2,FramingSize=2,MetadataLengthAdjustment=1;
inline constexpr std::size_t TrackDescriptorSize=387,SectorDescriptorSize=16,TrackHeaderSize=7;
inline constexpr std::size_t TrackCylinderOffset=0,TrackHeadOffset=1,TrackSectorCountOffset=2,MaximumDescriptors=23;
inline constexpr std::size_t SectorCylinderOffset=0,SectorHeadOffset=1,SectorNumberOffset=2,SectorSizeCodeOffset=3,SectorPositionOffset=5;
inline constexpr std::uint32_t BaseSectorSize=128,MaximumSizeCode=7,ReconstructedSectorSize=512,MaximumHeads=2;
inline constexpr std::uint32_t ByteBits=8;
inline constexpr std::size_t CylinderIndex=0,HeadIndex=1,NumberIndex=2;
}
