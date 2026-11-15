#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Coherent::Constants {
inline constexpr std::uint32_t BlockSize=512,MinimumImageSize=1024,InodeZoneEndOffset=512,BlockCountOffset=514,ModifiedTimeOffset=976,FreeBlockCountOffset=980,VolumeNameOffset=996,PackNameOffset=1002,NameLength=6;
inline constexpr std::uint32_t InodeSize=64,RootInode=2,MinimumInodeZoneEnd=3,ModeOffset=0,SizeOffset=8,PointersOffset=12,PointerSize=3,PointerCount=13,DirectCount=10,SingleIndex=10,DoubleIndex=11,TripleIndex=12,ModifiedOffset=56;
inline constexpr std::uint32_t CanonicalLength=4,CanonicalLow=2,CanonicalLowMiddle=3,CanonicalHighMiddle=0,CanonicalHigh=1,BitsPerByte=8,HighMiddleShift=16,HighShift=24,PointersPerBlock=128,PointerLow=1,PointerMiddle=2,PointerHigh=0,DoubleLevel=2,TripleLevel=3;
inline constexpr std::uint32_t DirectoryEntrySize=16,DirectoryNameLength=14,DirectoryInodeLength=2,MaximumDepth=64;
inline constexpr std::uint16_t DirectoryMode=0x4000,RegularMode=0x8000,TypeMask=0xf000,ProtectionMask=0x0fff;
inline constexpr std::uint32_t MaximumFileSize=2147483647,AsciiLimit=128;
inline constexpr char Replacement='?',Zero='\0',Space=' ',LineFeed='\n',Return='\r';
inline constexpr std::string_view FormatId="commodore900.coherent",FileSystemId="coherent",DefaultName="noname",DefaultPack="nopack",Placeholder="xxxxx",Current=".",Parent="..";
inline constexpr std::string_view InodePrefix="COHERENT inode ",InodeCommentKey="Media.CoherentInodeDescription",TranslationDomain="MediaFileSystems";
}
