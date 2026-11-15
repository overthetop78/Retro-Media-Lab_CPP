#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Rt11::Constants {
inline constexpr std::string_view Id="rt11",Format=Formats::DecRx02,Separator=", ",TranslationDomain="MediaFileSystems",TentativeKey="MediaFileSystems.Rt11Tentative",PermanentKey="MediaFileSystems.Rt11Permanent";
inline constexpr std::size_t MaximumStreamSize=std::size_t(std::numeric_limits<std::int32_t>::max());
inline constexpr std::uint8_t AsciiMaximum=0x7f,AsciiReplacement='?',UpperA='A',UpperZ='Z',LowerDelta='a'-'A';
inline constexpr char Space=' ',Null='\0',ExtensionSeparator='.';
inline constexpr std::int32_t BlockSize=512;
inline constexpr std::int32_t HomeBlock=1;
inline constexpr std::string_view SystemSignature="DECRT11";
inline constexpr std::int32_t DirectoryBlockOffset=468;
inline constexpr std::int32_t VolumeNameOffset=472;
inline constexpr std::int32_t VolumeNameLength=12;
inline constexpr std::int32_t SystemIdOffset=496;
inline constexpr std::int32_t SystemIdLength=12;
inline constexpr std::int32_t MinimumDirectoryBlock=2;
inline constexpr std::int32_t MaximumDirectoryBlockExclusive=1001;
inline constexpr std::int32_t SegmentBlockCount=2;
inline constexpr std::int32_t MaximumSegmentCount=31;
inline constexpr std::int32_t NextSegmentOffset=2;
inline constexpr std::int32_t ExtraBytesOffset=6;
inline constexpr std::int32_t DataBlockOffset=8;
inline constexpr std::int32_t EntriesOffset=10;
inline constexpr std::int32_t MinimumEntrySize=14;
inline constexpr std::int32_t MaximumEntrySize=128;
inline constexpr std::int32_t StatusOffset=0;
inline constexpr std::int32_t NameOffset=2;
inline constexpr std::int32_t ExtensionOffset=6;
inline constexpr std::int32_t BlockLengthOffset=8;
inline constexpr std::int32_t DateOffset=12;
inline constexpr std::uint32_t ProtectedAttribute=1;
inline constexpr std::uint32_t UnprotectedAttribute=0;
inline constexpr std::string_view TentativeFileDescription="Fichier RT-11 provisoire";
inline constexpr std::string_view PermanentFileDescription="Fichier RT-11";
inline constexpr std::string_view Alphabet=" ABCDEFGHIJKLMNOPQRSTUVWXYZ$.%0123456789";
inline constexpr std::int32_t Base=40;
inline constexpr std::int32_t CharacterCount=3;
inline constexpr std::int32_t FirstDivisor=Base * Base;
inline constexpr std::int32_t SecondDivisor=Base;
inline constexpr std::int32_t BaseYear=1972;
inline constexpr std::int32_t DayMask=0x1f;
inline constexpr std::int32_t MonthMask=0x0f;
inline constexpr std::int32_t MonthShift=5;
inline constexpr std::int32_t YearMask=0x1f;
inline constexpr std::int32_t YearShift=9;
inline constexpr std::int32_t AgeMask=0x03;
inline constexpr std::int32_t AgeShift=14;
inline constexpr std::int32_t YearsPerAge=32;
}
