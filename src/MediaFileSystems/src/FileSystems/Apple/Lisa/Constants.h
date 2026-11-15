#pragma once
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Lisa::Constants {
inline constexpr std::string_view Id="lisa",ImageId=Formats::AppleLisaOffice,DefaultVolumeName="Lisa",DefaultFilePrefix="Fichier Lisa $",FileDescriptionKey="MediaFileSystems.LisaFileDescription",TranslationDomain="MediaFileSystems";
inline constexpr std::string_view TableName="table",HashName="hash",BTreeName="B-tree";
inline constexpr std::string_view UnknownVersionPrefix="inconnue-0x";
inline constexpr std::uint16_t MddfFileId=0x0001,BitmapFileId=0x0002,SRecordsFileId=0x0003,CatalogFileId=0x0004,FreePageFileId=0x0000,AlternateFreePageFileId=0x7fff,FirstUserFileId=CatalogFileId+1,LastUserFileId=AlternateFreePageFileId-1;
inline constexpr std::array<std::uint16_t,4> ReservedFileIds{0x00aa,0x00bb,0xaaaa,0xbbbb};
inline constexpr std::size_t TagLength=12,TagFileIdHigh=4,TagPageHigh=6;
inline constexpr std::uint16_t PageNumberMask=0x07ff;
inline constexpr std::size_t TableEntrySize=54,TreeEntrySize=64,TreeEntriesOffset=0x50,CatalogNameOffset=1,CatalogNameLength=31,CatalogFileIdOffset=36;
inline constexpr std::uint8_t UnusedCatalogEntryMarker=0;
inline constexpr std::size_t MinimumMddfLength=Apple::RecognitionConstants::LisaMinimumLength,VersionOffset=Apple::RecognitionConstants::LisaVersionOffset,NameLengthOffset=Apple::RecognitionConstants::LisaNameLength,NameOffset=Apple::RecognitionConstants::LisaNameOffset,MaximumName=Apple::RecognitionConstants::LisaMaximumName;
inline constexpr std::uint8_t FirstPrintable=Apple::RecognitionConstants::PrintableFirst;
inline constexpr std::size_t MaximumBuffer=std::size_t(std::numeric_limits<std::int32_t>::max());
inline constexpr std::int32_t HexBase=16,HexWordWidth=4;
inline constexpr char HexZero='0';
}
