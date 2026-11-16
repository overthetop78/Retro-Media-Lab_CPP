#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Vmdk::Constants {
inline constexpr std::array<std::uint8_t,4> SparseSignature{0x4b,0x44,0x4d,0x56};
inline constexpr std::string_view DescriptorSignature="# Disk DescriptorFile",FormatId="hdd.vmdk",WriterId="hard-disk-vmdk",Extension=".vmdk",VariantKey="variant",FixedVariant="Fixed",SparseVariant="Sparse",DescriptorKey="descriptor",FlatSuffix="-flat";
inline constexpr std::uint32_t SparseMagic=0x564d444b,SparseVersion=1,ValidNewline=1,RedundantTables=2,ZeroedGrain=4,CompressedGrain=0x10000,MarkerFlag=0x20000,DefaultGrainSectors=128,DefaultTableEntries=512;
inline constexpr std::int32_t SectorSize=512,HeaderSize=512,SignatureSize=4;
inline constexpr std::uint64_t StreamDirectory=0xffffffffffffffffULL,MaximumSigned=0x7fffffffffffffffULL,MaximumField=0xffffffffULL,MaximumBuffer=0x7fffffff,MaximumDescriptor=4*1024*1024,DescriptorSectors=20,DescriptorSector=1;
inline constexpr std::size_t EntryBytes=4,VersionOffset=4,FlagsOffset=8,CapacityOffset=12,GrainSectorsOffset=20,DescriptorOffset=28,DescriptorSizeOffset=36,TableEntriesOffset=44,DirectoryOffset=56,OverheadOffset=64,UncleanOffset=72,NewlineOffset=73,SpaceOffset=74,CarriageOffset=75,LastNewlineOffset=76;
inline constexpr int DefaultHeads=16,DefaultSectors=63,FixedEnum=1,SparseEnum=4,ContentIdWidth=8;
inline constexpr char Quote='"',Space=' ',LineFeed='\n',CarriageReturn='\r',Null='\0';
inline constexpr char16_t Bom=0xfeff;
inline constexpr std::string_view LinePattern="[\r\n]",InitialWhitespace=" \t\r\n",DigitsPattern="^[0-9]+$",Assignment="=",ParentName="parentFileNameHint",CreateTypeName="createType",MonolithicFlat="monolithicFlat",MonolithicSparse="monolithicSparse",ReadWrite="RW",ReadOnly="RDONLY",NoAccess="NOACCESS",Flat="FLAT",Sparse="SPARSE",FlatExtentSuffix=" 0";
inline constexpr std::string_view DescriptorTemplate="# Disk DescriptorFile\nversion=1\nencoding=\"UTF-8\"\nCID=%1\nparentCID=ffffffff\ncreateType=\"%2\"\n\n# Extent description\nRW %3 %4 \"%5\"%6\n\n# The Disk Data Base\n#DDB\nddb.virtualHWVersion = \"4\"\nddb.geometry.cylinders = \"%7\"\nddb.geometry.heads = \"%8\"\nddb.geometry.sectors = \"%9\"\nddb.adapterType = \"lsilogic\"\n";
inline constexpr int ContentIdRadix=16;
inline constexpr int ExtentPrefixFields=3,ExtentTypeIndex=2;
inline constexpr char ContentIdPadding='0';
}
