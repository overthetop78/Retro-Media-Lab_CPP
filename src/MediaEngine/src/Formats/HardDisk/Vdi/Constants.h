#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Vdi::Constants {
inline constexpr std::uint32_t Signature=0xbeda107f,Version=0x00010001,DynamicType=1,FixedType=2,Unallocated=0xffffffff,Discarded=0xfffffffe;
inline constexpr std::size_t HeaderSize=512,DeclaredHeaderSize=0x180,DefaultBlockSize=1024*1024,EntryBytes=4,GuidBytes=16,ByteBits=8,DoubleWordBytes=8;
inline constexpr std::size_t UuidMiddleOffset=4,UuidLastOffset=6;
inline constexpr int FixedEnumValue=1,DynamicEnumValue=2;
inline constexpr std::int32_t SectorSize=512;
inline constexpr std::uint64_t MaximumLogicalSize=0x7fffffffffffffffULL,MaximumSignedField=0x7fffffff,MaximumUnsignedField=0xffffffff;
inline constexpr std::size_t SignatureOffset=0x40,VersionOffset=0x44,HeaderSizeOffset=0x48,ImageTypeOffset=0x4c,DescriptionOffset=0x54,MapOffset=0x154,DataOffset=0x158,CylindersOffset=0x15c,HeadsOffset=0x160,SectorsOffset=0x164,SectorSizeOffset=0x168,DiskSizeOffset=0x170,BlockSizeOffset=0x178,BlockExtraOffset=0x17c,BlocksCountOffset=0x180,BlocksAllocatedOffset=0x184,ImageUuidOffset=0x188,ModifyUuidOffset=0x198,ParentUuidOffset=0x1b8;
inline constexpr std::string_view FormatId="hdd.vdi",WriterId="hard-disk-vdi",Extension=".vdi",VariantKey="variant",UuidKey="imageUuid",FixedVariant="Fixed",DynamicVariant="Dynamic";
inline constexpr std::string_view Banner="<<< Retro Media Lab Virtual Disk Image >>>\n",Description="Retro Media Lab MediaEngine";
}
