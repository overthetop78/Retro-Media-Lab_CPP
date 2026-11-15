#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Ucsd::Constants {
inline constexpr std::string_view Id="ucsd",System="UCSD p-System",Format=Formats::UcsdIbmMfm,Separator=", ",TranslationDomain="MediaFileSystems";
inline constexpr std::uint8_t UpperA='A',UpperZ='Z',LowerDelta='a'-'A';
inline constexpr std::size_t MaximumStreamSize=std::size_t(std::numeric_limits<std::int32_t>::max());
inline constexpr std::array<std::string_view,9> TypeNames={"UCSD untyped file","UCSD external disk file","UCSD code file","UCSD text file","UCSD info file","UCSD data file","UCSD graphics file","UCSD photo file","UCSD secure directory"};
inline constexpr std::array<std::string_view,9> TypeKeys={"MediaFileSystems.UcsdTypeUntyped","MediaFileSystems.UcsdTypeExternalDisk","MediaFileSystems.UcsdTypeCode","MediaFileSystems.UcsdTypeText","MediaFileSystems.UcsdTypeInfo","MediaFileSystems.UcsdTypeData","MediaFileSystems.UcsdTypeGraphics","MediaFileSystems.UcsdTypePhoto","MediaFileSystems.UcsdTypeSecureDirectory"};
inline constexpr std::int32_t BlockSize=512;
inline constexpr std::int32_t DirectoryBlock=2;
inline constexpr std::int32_t EntrySize=26;
inline constexpr std::int32_t ShortDirectoryEnd=6;
inline constexpr std::int32_t LongDirectoryEnd=10;
inline constexpr std::int32_t ShortDirectoryBlockCount=4;
inline constexpr std::int32_t LongDirectoryBlockCount=8;
inline constexpr std::int32_t MaximumVolumeNameLength=7;
inline constexpr std::int32_t MaximumFileCount=77;
inline constexpr std::uint8_t MinimumNameCharacter=0x20;
inline constexpr std::uint8_t MaximumNameCharacterExclusive=0x7f;
inline constexpr std::int32_t DirectoryEndOffset=2;
inline constexpr std::int32_t VolumeNameOffset=6;
inline constexpr std::int32_t VolumeNameFieldLength=8;
inline constexpr std::int32_t TotalBlocksOffset=14;
inline constexpr std::int32_t FileCountOffset=16;
inline constexpr std::int32_t VolumeDateOffset=20;
inline constexpr std::int32_t EntryFirstBlockOffset=0;
inline constexpr std::int32_t EntryLastBlockOffset=2;
inline constexpr std::int32_t EntryKindOffset=4;
inline constexpr std::int32_t EntryNameOffset=6;
inline constexpr std::int32_t EntryNameFieldLength=16;
inline constexpr std::int32_t MaximumFileNameLength=15;
inline constexpr std::int32_t EntryLastBlockBytesOffset=22;
inline constexpr std::int32_t EntryDateOffset=24;
inline constexpr std::int32_t FileKindMask=0x0f;
inline constexpr std::int32_t DayMask=0x1f;
inline constexpr std::int32_t MonthMask=0x0f;
inline constexpr std::int32_t MonthShift=5;
inline constexpr std::int32_t YearMask=0x7f;
inline constexpr std::int32_t YearShift=9;
inline constexpr std::int32_t CenturyPivot=70;
inline constexpr std::int32_t NineteenthCenturyBase=1900;
inline constexpr std::int32_t TwentiethCenturyBase=2000;
}
