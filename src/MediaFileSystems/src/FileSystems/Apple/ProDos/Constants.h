#pragma once
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::ProDos::Constants {
namespace Common=Apple::RecognitionConstants;
inline constexpr std::uint32_t BlockSize=Common::BlockSize,RootBlock=Common::ProDosRootBlock;
inline constexpr std::size_t HeaderOffset=Common::ProDosHeader,HeaderEntryLengthOffset=Common::ProDosEntryLength,EntrySize=Common::ProDosEntrySize;
inline constexpr std::uint32_t RootDirectoryBlockCount=4,DefaultBitmapBlock=6,EntriesPerDirectoryBlock=13,IndexPointerCount=256,IndexHighBytesOffset=256,IndexHalfLength=256,BlocksPerBitmapBlock=4096;
inline constexpr std::size_t StorageAndNameLengthOffset=0,NameOffset=1,FileTypeOffset=16,KeyBlockOffset=17,BlocksUsedOffset=19,EndOfFileOffset=21,CreatedDateOffset=24,AccessOffset=30,ModifiedDateOffset=33,HeaderPointerOffset=37;
inline constexpr std::size_t SubdirectoryReservedOffset=HeaderOffset+16,SubdirectoryVersionOffset=HeaderOffset+28,SubdirectoryParentBlockOffset=HeaderOffset+35,SubdirectoryParentEntryOffset=HeaderOffset+37,SubdirectoryParentEntryLengthOffset=HeaderOffset+38;
inline constexpr std::size_t PreviousBlockOffset=0,NextBlockOffset=2,FirstVolumeEntryOffset=HeaderOffset+EntrySize,FirstChainedEntryOffset=HeaderOffset,BitmapBlockOffset=HeaderOffset+35,TotalBlocksOffset=HeaderOffset+37;
inline constexpr std::uint8_t SubdirectoryReservedValue=0x76,SubdirectoryVersion=5,DefaultAccess=0xc3,BitmapHighBitMask=0x80,NameLengthMask=Common::ProDosNameMask,StorageTypeShift=Common::ProDosStorageShift;
inline constexpr std::uint32_t MaximumFileLength=0x00ffffff,MaximumDirectoryDepth=64,BitsPerByte=8,MaximumNameLength=15;
inline constexpr std::uint32_t MaximumWord=std::numeric_limits<std::uint16_t>::max();
inline constexpr std::uint8_t UpperFirst='A',UpperLast='Z',LowerFirst='a',LowerLast='z',DigitFirst='0',DigitLast='9',AsciiReplacement='?',Dot='.';
inline constexpr auto CaseDelta=LowerFirst-UpperFirst;
inline constexpr std::int32_t BaseYear=1900,PivotYear=1940,YearShift=9,MonthShift=5,MonthMask=0x0f,DayMask=0x1f,HourShift=8,MinuteMask=0x3f,YearCentury=100,DateTimeLength=4;
inline constexpr std::string_view Id="prodos",SosId="apple3-sos",Domain="MediaFileSystems";
inline constexpr auto ProDos=Formats::AppleIIProDos,ProDos140=Formats::AppleIIProDos140,ProDos800=Formats::AppleIIProDos800,Sos=Formats::AppleIIISos;
inline constexpr std::array<std::uint8_t,6> Types{0x04,0x06,0x0f,0xfc,0xfd,0xff};
inline constexpr std::array<std::string_view,6> TypeNames{"Text","Binary","Directory","BASIC","Variables","System"};
inline constexpr std::array<std::string_view,7> TypeKeys{"MediaFileSystems.ProDosTypeText","MediaFileSystems.ProDosTypeBinary","MediaFileSystems.ProDosTypeDirectory","MediaFileSystems.ProDosTypeBasic","MediaFileSystems.ProDosTypeVariables","MediaFileSystems.ProDosTypeSystem","MediaFileSystems.ProDosTypeUnknown"};
inline constexpr std::string_view UnknownTypePrefix="ProDOS $",True="True",False="False",SeedlingName="Seedling",SaplingName="Sapling",TreeName="Tree";
inline constexpr char HexDigits[]="0123456789ABCDEF";
inline constexpr std::size_t HexByteDigits=2;
inline constexpr std::uint8_t HexMask=0x0f,HexShift=4;
}
