#pragma once
#include <RetroMediaLab/MediaFileSystems/FamicomDiskMetadataConstants.h>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::FamicomDiskConstants {
inline constexpr int DiskInfoBlockCode=0x01, DiskInfoLength=56, FileCountBlockCode=0x02, FileCountLength=2, FileHeaderBlockCode=0x03, FileHeaderLength=18, FileDataBlockCode=0x04;
inline constexpr int FileNumberOffset=1, FileIdentifierOffset=2, FileIdentifierLength=3, FileNameOffset=5, FileNameLength=8, FileAddressOffset=13, FileSizeOffset=15, FileTypeOffset=17, NameDigits=2;
inline constexpr char Null='\0', Space=' ', Zero='0', AsciiReplacement='?';
inline constexpr unsigned AsciiMaximum=127;
inline constexpr std::string_view Id="nintendo-fds", DisplayName="Famicom Disk System", DefaultFileName="file", SideMetadata="side", FileNumberMetadata="fileNumber", FileAddressMetadata="address", FileTypeMetadata="type", FileEntryType="fds-file", SideAttribute="side", FileAttribute="file", FileIdentifier="HVC", SideCount=FamicomDiskMetadataConstants::SideCount, SideLength=FamicomDiskMetadataConstants::SideLength;
}
