#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Vhdx::Constants {
inline constexpr std::size_t GuidLength=16;
using Guid=std::array<std::uint8_t,GuidLength>;
inline constexpr std::uint64_t FirstSequence=1,SecondSequence=2;
inline constexpr std::uint32_t BlockSizeGrowth=2;
inline constexpr std::string_view FileSignature="vhdxfile",HeaderSignature="head",RegionSignature="regi",MetadataSignature="metadata",Creator="Retro Media Lab MediaEngine",FormatId="hdd.vhdx",WriterId="hard-disk-vhdx",Extension=".vhdx",VariantKey="variant",FixedVariant="Fixed",DynamicVariant="Dynamic",Page83Key="page83Id",PhysicalSectorKey="physicalSectorSize";
inline constexpr std::uint32_t MiB=1024*1024,DefaultBlockSize=32*MiB,MaximumBlockSize=256*MiB,LegacySector=512,LargeSector=4096,LeaveAllocated=1,HasParent=2,MaximumEntries=2047,RegionRequired=1,MetadataRequired=4,MetadataVirtualRequired=6;
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffffULL,MaximumField=0xffffffffULL,MaximumBuffer=0x7fffffff,StateMask=7,FileOffsetMask=0xfffffffffff00000ULL,NotPresent=0,Undefined=1,Zero=2,Unmapped=3,FullyPresent=6,PartiallyPresent=7,BitmapSectors=1ULL<<23,MetadataRegionOffset=MiB,BatRegionOffset=2*MiB;
inline constexpr std::size_t IdentifierSize=64*1024,HeaderSize=4*1024,RegionTableSize=64*1024,MetadataTableSize=64*1024,Header1Offset=64*1024,Header2Offset=128*1024,Region1Offset=192*1024,Region2Offset=256*1024,GuidBytes=16,GuidMiddleOffset=4,GuidLastOffset=6,EntryBytes=8,ChecksumOffset=4,ChecksumSize=4,SequenceOffset=8,FileWriteIdOffset=16,DataWriteIdOffset=32,LogIdOffset=48,HeaderVersionOffset=66,LogLengthOffset=68,LogOffset=72,RegionCountOffset=8,RegionEntriesOffset=16,StructureEntrySize=32,RegionFileOffset=16,RegionLengthOffset=24,RegionFlagsOffset=28,MetadataCountOffset=10,MetadataEntriesOffset=32,MetadataItemOffset=16,MetadataItemLengthOffset=20,MetadataItemFlagsOffset=24,MetadataValueOffset=64*1024,ParametersSize=8,ParametersFlagsOffset=4,DiskSizeBytes=8,SectorSizeBytes=4,MetadataEntryCount=5,RegionEntryCount=2,CreatorOffset=8,Utf16Bytes=2;
inline constexpr std::uint16_t Version=1;
inline constexpr std::array<std::size_t,5> MetadataValueLengths{ParametersSize,DiskSizeBytes,GuidBytes,SectorSizeBytes,SectorSizeBytes};
inline constexpr std::array<std::uint32_t,5> MetadataValueFlags{MetadataRequired,MetadataVirtualRequired,MetadataVirtualRequired,MetadataVirtualRequired,MetadataVirtualRequired};
// GUID bytes use the on-disk .NET/Windows little-endian field order.
inline constexpr Guid BatGuid{0x66,0x77,0xc2,0x2d,0x23,0xf6,0x00,0x42,0x9d,0x64,0x11,0x5e,0x9b,0xfd,0x4a,0x08};
inline constexpr Guid MetadataGuid{0x06,0xa2,0x7c,0x8b,0x90,0x47,0x9a,0x4b,0xb8,0xfe,0x57,0x5f,0x05,0x0f,0x88,0x6e};
inline constexpr Guid FileParametersGuid{0x37,0x67,0xa1,0xca,0x36,0xfa,0x43,0x4d,0xb3,0xb6,0x33,0xf0,0xaa,0x44,0xe7,0x6b};
inline constexpr Guid DiskSizeGuid{0x24,0x42,0xa5,0x2f,0x1b,0xcd,0x76,0x48,0xb2,0x11,0x5d,0xbe,0xd8,0x3b,0xf4,0xb8};
inline constexpr Guid Page83Guid{0xab,0x12,0xca,0xbe,0xe6,0xb2,0x23,0x45,0x93,0xef,0xc3,0x09,0xe0,0x00,0xc7,0x46};
inline constexpr Guid LogicalSectorGuid{0x1d,0xbf,0x41,0x81,0x6f,0xa9,0x09,0x47,0xba,0x47,0xf2,0x33,0xa8,0xfa,0xab,0x5f};
inline constexpr Guid PhysicalSectorGuid{0xc7,0x48,0xa3,0xcd,0x5d,0x44,0x71,0x44,0x9c,0xc9,0xe9,0x88,0x52,0x51,0xc5,0x56};
inline constexpr std::array<Guid,5> MetadataIds{FileParametersGuid,DiskSizeGuid,Page83Guid,LogicalSectorGuid,PhysicalSectorGuid};
}
