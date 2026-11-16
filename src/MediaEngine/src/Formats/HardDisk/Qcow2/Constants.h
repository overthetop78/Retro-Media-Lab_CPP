#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Qcow2::Constants {
inline constexpr std::array<std::uint8_t,4> Signature{0x51,0x46,0x49,0xfb};
inline constexpr std::uint32_t Magic=0x514649fb,Version2=2,Version3=3,MinimumClusterBits=9,MaximumClusterBits=21,DefaultClusterBits=16,DefaultRefcountOrder=4;
inline constexpr std::uint64_t CopiedFlag=0x8000000000000000ULL,CompressedFlag=0x4000000000000000ULL,ZeroFlag=1,ClusterOffsetMask=0x00fffffffffffe00ULL,KnownIncompatibleFeatures=0,MaximumSignedLength=0x7fffffffffffffffULL,MaximumField=0xffffffffULL,MaximumBuffer=0x7fffffff;
inline constexpr std::int32_t SectorSize=512;
inline constexpr std::size_t Header2Size=72,Header3Size=104,EntryBytes=8,RefcountBytes=2;
inline constexpr std::size_t SignatureOffset=0,VersionOffset=4,BackingOffset=8,BackingSizeOffset=16,ClusterBitsOffset=20,VirtualSizeOffset=24,EncryptionOffset=32,L1SizeOffset=36,L1Offset=40,RefcountOffset=48,RefcountClustersOffset=56,SnapshotCountOffset=60,SnapshotOffset=64,IncompatibleOffset=72,AutoclearOffset=88,RefcountOrderOffset=96,HeaderLengthOffset=100;
inline constexpr std::string_view FormatId="hdd.qcow2",WriterId="hard-disk-qcow2",Extension=".qcow2",AlternateExtension=".qcow",VersionKey="version",VariantKey="variant",SparseVariant="Sparse",ClusterSizeKey="clusterSize",L1Name="L1",L2DataName="L2 data",L1TableName="L1 table",RefcountTableName="refcount table",ReferenceName="referenced",RefcountBlockName="refcount block";
}
