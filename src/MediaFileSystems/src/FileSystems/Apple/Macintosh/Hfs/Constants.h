#pragma once
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::MacHfs::Constants {
inline constexpr std::string_view Id="mac-hfs",SystemName="Macintosh HFS",DirectoryDescription="Dossier",DefaultFileDescription="Fichier Macintosh";
inline constexpr std::string_view DirectoryKey="MediaFileSystems.HfsDirectory",DefaultFileKey="MediaFileSystems.MfsDefaultFile",TranslationDomain="MediaFileSystems";
inline constexpr std::int32_t HexBase=16,HexWordWidth=4;
inline constexpr char HexZero='0';
inline constexpr std::size_t RecordEndOffsetEntries=2;
inline constexpr std::size_t MaximumStreamSize=std::size_t(std::numeric_limits<std::int32_t>::max());
inline constexpr std::int32_t SectorSize=512;
inline constexpr std::int32_t MasterDirectoryBlock=2;
inline constexpr std::int32_t MinimumMdbLength=162;
inline constexpr std::uint16_t Signature=Apple::RecognitionConstants::HfsSignature;
inline constexpr std::int32_t CreatedOffset=2;
inline constexpr std::int32_t ModifiedOffset=6;
inline constexpr std::int32_t AllocationCountOffset=18;
inline constexpr std::int32_t AllocationSizeOffset=20;
inline constexpr std::int32_t AllocationStartOffset=28;
inline constexpr std::int32_t FreeAllocationCountOffset=34;
inline constexpr std::int32_t VolumeNameOffset=36;
inline constexpr std::int32_t MaximumVolumeNameLength=27;
inline constexpr std::int32_t CatalogLengthOffset=146;
inline constexpr std::int32_t CatalogExtentsOffset=150;
inline constexpr std::int32_t EmbeddedExtentsLength=12;
inline constexpr std::int32_t EmbeddedExtentCount=3;
inline constexpr std::int32_t ExtentDescriptorLength=4;
inline constexpr std::int32_t ExtentStartOffset=0;
inline constexpr std::int32_t ExtentCountOffset=2;
inline constexpr std::int32_t MinimumCatalogLength=64;
inline constexpr std::int32_t NodeSizeOffset=32;
inline constexpr std::int32_t DefaultNodeSize=SectorSize;
inline constexpr std::int32_t MinimumNodeSize=256;
inline constexpr std::int32_t MaximumNodeSize=32768;
inline constexpr std::int32_t NodeKindOffset=8;
inline constexpr std::int8_t LeafNodeKind=-1;
inline constexpr std::int32_t RecordCountOffset=10;
inline constexpr std::int32_t NodeDescriptorLength=14;
inline constexpr std::int32_t MaximumRecordCount=512;
inline constexpr std::int32_t RecordOffsetLength=sizeof(std::uint16_t);
inline constexpr std::int32_t MinimumKeyLength=6;
inline constexpr std::int32_t ParentIdOffset=1;
inline constexpr std::int32_t NameLengthOffset=5;
inline constexpr std::int32_t NameOffset=6;
inline constexpr std::int32_t MaximumNameLength=31;
inline constexpr std::int32_t RecordAlignment=2;
inline constexpr std::uint8_t DirectoryRecordType=1;
inline constexpr std::uint8_t FileRecordType=2;
inline constexpr std::int32_t MinimumDirectoryRecordLength=70;
inline constexpr std::int32_t DirectoryIdOffset=6;
inline constexpr std::int32_t DirectoryModifiedOffset=14;
inline constexpr std::int32_t MinimumFileRecordLength=102;
inline constexpr std::int32_t FinderTypeOffset=4;
inline constexpr std::int32_t FinderTypeLength=4;
inline constexpr std::int32_t FileIdOffset=20;
inline constexpr std::int32_t DataForkLengthOffset=26;
inline constexpr std::int32_t ResourceForkLengthOffset=36;
inline constexpr std::int32_t FileModifiedOffset=48;
inline constexpr std::int32_t DataForkExtentsOffset=74;
inline constexpr std::int32_t ResourceForkExtentsOffset=86;
inline constexpr std::uint32_t RootDirectoryId=2;
}
