#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Vhd::Constants {
inline constexpr std::string_view FormatId="hdd.vhd",WriterId="hard-disk-vhd",Extension=".vhd",FooterCookie="conectix",DynamicCookie="cxsparse",VariantKey="variant",UuidKey="uniqueId",FixedVariant="Fixed",DynamicVariant="Dynamic",Creator="GWGU";
inline constexpr std::size_t FooterSize=512,DynamicHeaderSize=1024,SectorSize=512,DefaultBlockSize=2*1024*1024,EntryBytes=4,GuidBytes=16,ByteBits=8;
inline constexpr std::uint32_t FixedType=2,DynamicType=3,DifferencingType=4,UnusedEntry=0xffffffff,Version=0x00010000,FooterFeatures=2,CreatorHostOs=0x5769326b;
inline constexpr std::uint64_t MaximumLogicalSize=0x7fffffffffffffffULL,MaximumSignedField=0x7fffffff,MaximumUnsignedField=0xffffffff,NoDataOffset=0xffffffffffffffffULL;
inline constexpr std::int64_t EpochUnixSeconds=946684800;
inline constexpr std::size_t FeaturesOffset=8,VersionOffset=12,DataOffset=16,TimestampOffset=24,CreatorOffset=28,CreatorVersionOffset=32,CreatorHostOffset=36,OriginalSizeOffset=40,CurrentSizeOffset=48,CylindersOffset=56,HeadsOffset=58,SectorsOffset=59,TypeOffset=60,ChecksumOffset=64,UuidOffset=68;
inline constexpr std::size_t DynamicNextOffset=8,TableOffset=16,DynamicVersionOffset=24,MaximumEntriesOffset=28,BlockSizeOffset=32,DynamicChecksumOffset=36,DynamicTableOffset=FooterSize+DynamicHeaderSize;
inline constexpr std::uint8_t BitmapFirstBit=0x80;
inline constexpr std::int64_t MaximumCylinders=65535,MaximumHeads=16,MaximumSectors=255,GeometryThresholdSectors=63,InitialSectors=17,MiddleSectors=31,MinimumHeads=4,CylinderThreshold=1024;
}
