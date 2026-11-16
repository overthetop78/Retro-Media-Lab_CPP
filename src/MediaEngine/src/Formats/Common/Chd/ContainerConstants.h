#pragma once
#include "CodecConstants.h"
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Chd::Constants {
inline constexpr std::string_view Signature="MComprHD";
inline constexpr std::string_view Cookie{"EndOfListCookie\0",16};
inline constexpr int HeaderBytes=124,SignatureBytes=8,HeaderSizeOffset=8,VersionOffset=12,Version5=5,Version3=3,Version1=1,CodecCount=4,WordBytes=4,CodecOffset=16,LogicalOffset=32,MapOffset=40,MetadataOffset=48,HunkOffset=56,UnitOffset=60,ParentOffset=104,ParentBytes=20;
inline constexpr std::array<int,6> HeaderSizes{0,76,80,120,108,124};
inline constexpr int LegacyCodecOffset=20,LegacyFlagsOffset=16,LegacyCountOffset=24,OldCountOffset=28,OldSectorsOffset=24,OldCylindersOffset=32,OldHeadsOffset=36,OldTracksOffset=40,OldSectorOffset=76,LegacyLogicalOffset=28,LegacyMetadataOffset=36,V3HunkOffset=76,V4HunkOffset=44,SectorBytes=512;
inline constexpr std::uint64_t MaximumLogical=10ull*1024*1024*1024,MaximumMap=0x7fffffff,OldOffsetMask=0x00000fffffffffff;
inline constexpr int MaximumLegacyCodec=2,ParentFlag=1,OldEntryBytes=8,LegacyEntryBytes=16,OldLengthShift=44,LegacyLengthOffset=12,LegacyLengthHigh=14,LegacyTypeOffset=15,LegacyTypeMask=15,ByteBits=8,WordBits=16,LongBits=24;
inline constexpr int UncompressedMapBytes=4,MapEntryBytes=12,MapHeaderBytes=16,MapLengthOffset=0,MapDataOffset=4,MapCrcOffset=10,LengthBitsOffset=12,SelfBitsOffset=13,ParentBitsOffset=14,MapSymbols=16,MapCodeBits=8,PollMask=1023;
inline constexpr int CodecThird=2,CodecLast=3,RawType=4,SelfType=5,ParentType=6,SmallRun=7,LargeRun=8,SmallRunBias=2,LargeRunBias=18,RunShift=4,SelfLast=9,SelfNext=10,ParentCurrent=11,ParentLast=12,ParentNext=13,EntryLengthOffset=1,EntryDataOffset=4,EntryCrcOffset=10;
inline constexpr int LegacyCompressed=1,LegacyRaw=2,LegacyMini=3,LegacySelf=4,PatternBytes=8,PatternMask=7,MetadataHeaderBytes=16,MetadataTagOffset=0,MetadataLengthOffset=4,MetadataNextOffset=8,TagBytes=4;
inline constexpr std::uint32_t MetadataLengthMask=0xffffff,Zlib=0x7a6c6962,Lzma=0x6c7a6d61,HuffmanCodec=0x68756666,FlacCodec=0x666c6163,Zstd=0x7a737464,CdZlib=0x63647a6c,CdLzma=0x63646c7a,CdFlac=0x6364666c,CdZstd=0x63647a73;
inline constexpr int HuffmanSymbols=256,HuffmanMaximumBits=16,FlacAlignment=4,CdBytes=2448,CdMainBytes=2352,CdSubBytes=96,CdLengthBoundary=65536,CdShortLength=2,CdLongLength=3,EccByteBits=8,EccByteMask=7,EccByteShift=3,CdModeOffset=15,CdAddressBytes=4,CdDataOffset=12,ParityPCount=86,ParityQCount=52,ParityPRow=24,ParityQRow=43,ParityPOffset=0x81c,ParityQOffset=0x8c8;
inline constexpr std::uint8_t LittleEndian='L',BigEndian='B',Mode2=2;
inline constexpr std::array<std::uint8_t,12> CdSync{0,255,255,255,255,255,255,255,255,255,255,0};
inline constexpr std::string_view LegacyCodecName="zlib";
inline constexpr char CodecSeparator=',',AsciiReplacement='?';
inline constexpr std::uint8_t AsciiLimit=128;
}
