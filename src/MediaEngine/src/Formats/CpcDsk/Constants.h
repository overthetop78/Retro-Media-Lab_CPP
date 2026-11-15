#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaEngine::CpcDsk::Constants {
inline constexpr std::string_view WriterId="amstrad-dsk";
inline constexpr std::string_view FormatId=MediaFileSystems::Formats::CpcEmuDsk,StandardHeader="MV - CPCEMU Disk-File\r\nDisk-Info\r\n",StandardSignature="MV - CPC",ExtendedHeader="EXTENDED CPC DSK File\r\nDisk-Info\r\n",ExtendedSignature="EXTENDED CPC DSK File",TrackHeader="Track-Info\r\n",TrackSignature="Track-Info",Creator="Retro Media Lab";
inline constexpr std::array<std::string_view,2> Extensions{".dsk",".edsk"};
inline constexpr std::size_t DiskInformationBlockSize=0x100,TrackInformationBlockSize=0x100,DiskSignatureLength=34,CreatorOffset=34,CreatorLength=14,CylinderCountOffset=48,HeadCountOffset=49,StandardTrackSizeOffset=50,ExtendedTrackSizeTableOffset=52,ExtendedTrackSizeUnit=256;
inline constexpr std::uint32_t MaximumCylinderCount=168,MinimumCylinderCount=1,MaximumHeadCount=2,MinimumHeadCount=1,MinimumSectorSize=128,MinimumSectorsPerTrack=1,SectorSizeCodeMask=0x07;
inline constexpr std::size_t TrackSignatureLength=12,TrackCylinderOffset=16,TrackHeadOffset=17,TrackSectorSizeCodeOffset=20,TrackSectorCountOffset=21,TrackGap3LengthOffset=22,TrackFillerByteOffset=23,SectorDescriptorTableOffset=24,SectorDescriptorSize=8,SectorCylinderOffset=0,SectorHeadOffset=1,SectorIdOffset=2,SectorSizeCodeOffset=3,SectorStatus1Offset=4,SectorStatus2Offset=5,SectorStoredSizeOffset=6,StoredSizeFieldLength=2,MaximumSectorsPerTrack=(TrackInformationBlockSize-SectorDescriptorTableOffset)/SectorDescriptorSize;
inline constexpr std::uint8_t DataErrorMask=0x20,DeletedDataMarkMask=0x40,DefaultGap3Length=0x4e,DefaultFillerByte=0xe5;
inline constexpr std::size_t NativeMetadataLength=2,NativeStatus1Offset=0,NativeStatus2Offset=1,MaximumBuffer=std::size_t(std::numeric_limits<std::int32_t>::max());
}
