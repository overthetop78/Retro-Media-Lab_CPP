#pragma once
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::MacMfs::Constants {
inline constexpr std::string_view Id="mac-mfs",System="Macintosh MFS",DefaultDescription="Fichier Macintosh",DefaultDescriptionKey="MediaFileSystems.MfsDefaultFile",TranslationDomain="MediaFileSystems";
inline constexpr std::int32_t SectorSize=512,MasterBlock=2,VolumeBlocks=2,MinimumMdb=64,MaximumVolumeName=27;
inline constexpr std::uint16_t Signature=Apple::RecognitionConstants::MfsSignature;
inline constexpr std::size_t Created=2,Modified=6,DirectoryStart=14,DirectoryLength=16,AllocationCount=18,AllocationSize=20,AllocationStart=28,FreeAllocationCount=34,VolumeName=36,MapOffset=64,MapLength=960;
inline constexpr std::int32_t MaximumAllocations=640,PackedPairLength=3,BitsPerAllocation=12,HalfByteShift=4,ByteShift=8,FirstCluster=2,EndOfChain=0x0ff1;
inline constexpr std::uint16_t AllocationMask=0x0fff,LowNibble=0x0f;
inline constexpr std::int32_t EntriesPerPackedPair=2,HexBase=16,HexWordWidth=4;
inline constexpr char HexZero='0';
inline constexpr std::size_t MinimumEntry=51,Flags=0,FinderInfo=2,FinderLength=16,FinderTypeLength=4,FileNumber=18,DataStart=22,DataLength=24,DataPhysicalLength=28,ResourceStart=32,ResourceLength=34,ResourcePhysicalLength=38,CreatedDate=42,ModifiedDate=46,NameLength=50,Name=51,MaximumName=63,Alignment=2;
inline constexpr std::uint8_t Active=0x80;
inline constexpr std::size_t MaximumStreamSize=std::size_t(std::numeric_limits<std::int32_t>::max());
}
