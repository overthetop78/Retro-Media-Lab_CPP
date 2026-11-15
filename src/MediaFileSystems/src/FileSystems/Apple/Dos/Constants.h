#pragma once
#include <array>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::AppleDos::Constants {
inline constexpr std::string_view Id="apple-dos",Domain="MediaFileSystems",VolumePrefix="DOS-",CatalogName="catalog";
inline constexpr std::string_view Dos32=Formats::AppleIIDos32,Dos33=Formats::AppleIIDos33,Dos113=Formats::AppleIIAppleDos113,Dos140=Formats::AppleIIAppleDos140;
inline constexpr std::int32_t SectorSize=256,TrackCount=35,Dos32Sectors=13,Dos33Sectors=16,VtocTrack=17;
inline constexpr std::size_t CatalogTrack=1,CatalogSector=2,VolumeNumber=6,MaximumPairs=0x27,LastAllocated=0x30,AllocationDirection=0x31,Tracks=0x34,TrackSectors=0x35,SectorBytes=0x36,FreeBitmap=0x38,BitmapSize=4;
inline constexpr std::size_t CatalogFirst=0x0b,EntrySize=35,EntriesPerSector=7,EntryTrack=0,EntrySector=1,EntryType=2,EntryName=3,NameLength=30,EntryCount=33;
inline constexpr std::size_t PairOffset=0x0c,PairSize=2,PairCount=122,ListOffset=5,NextTrack=1,NextSector=2;
inline constexpr std::size_t BinaryHeader=4,BinaryAddress=0,BinaryLength=2,BinaryAddressShift=8;
inline constexpr std::uint8_t ValueMask=0x7f,Deleted=0xff,Unused=0,Locked=0x80,BinaryType=4,HighBit=0x80,NamePadding=0xa0,Dos32Version=2,Dos33Version=4,Descending=0xff;
inline constexpr std::uint32_t BitmapBits=32;
inline constexpr std::size_t MaximumBinarySize=65535,VolumeDigits=3,VolumeTextLength=7;
inline constexpr std::uint32_t DecimalBase=10;
inline constexpr std::uint8_t AsciiReplacement='?';
inline constexpr std::uint8_t MaximumVolume=255;
inline constexpr char Space=' ',Null='\0',Comma=',',NameFirst='@',PrintableLast='~',DigitFirst='0',DigitLast='9',Plus='+',Minus='-',LowerFirst='a',LowerLast='z',UpperFirst='A',UpperLast='Z',CaseDelta='a'-'A';
inline constexpr std::uint16_t ParseWhiteFirst=9,ParseWhiteLast=13;
inline constexpr std::array<std::uint8_t,8> Types{0,1,2,4,8,16,32,64};
inline constexpr std::array<std::string_view,9> TypeNames{"Text","Integer BASIC","Applesoft BASIC","Binary","S","Relocatable","A","B","File"};
inline constexpr std::array<std::string_view,9> TypeKeys{"MediaFileSystems.AppleDosTypeText","MediaFileSystems.AppleDosTypeIntegerBasic","MediaFileSystems.AppleDosTypeApplesoftBasic","MediaFileSystems.AppleDosTypeBinary","MediaFileSystems.AppleDosTypeS","MediaFileSystems.AppleDosTypeRelocatable","MediaFileSystems.AppleDosTypeA","MediaFileSystems.AppleDosTypeB","MediaFileSystems.AppleDosTypeFile"};
}
