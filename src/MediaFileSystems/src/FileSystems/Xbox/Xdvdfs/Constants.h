#pragma once
#include <cstdint>
#include <string_view>
#include <array>
namespace RetroMediaLab::MediaFileSystems::XdvdfsConstants {
inline constexpr std::string_view FileSystemDisplayName="Xbox XDVDFS";
inline constexpr std::string_view FileAttribute="XDVDFS";
inline constexpr std::string_view FileEntryType="xdvdfs-file";
inline constexpr std::string_view DirectoryEntryType="xdvdfs-directory";
inline constexpr std::string_view SectorMetadata="sector";
inline constexpr std::string_view LengthMetadata="length";
inline constexpr int SectorSize=2048;
inline constexpr int VolumeDescriptorSector=32;
inline constexpr int VolumeDescriptorLength=SectorSize;
inline constexpr int MagicOffset=0;
inline constexpr int RootDirectorySectorOffset=0x14;
inline constexpr int RootDirectorySizeOffset=0x18;
inline constexpr int DescriptorTrailerOffset=0x7EC;
inline constexpr int LeftChildOffset=0x00;
inline constexpr int RightChildOffset=0x02;
inline constexpr int FileStartSectorOffset=0x04;
inline constexpr int FileLengthOffset=0x08;
inline constexpr int FileAttributesOffset=0x0C;
inline constexpr int FileNameLengthOffset=0x0D;
inline constexpr int FileNameOffset=0x0E;
inline constexpr int MinimumEntryLength=FileNameOffset;
inline constexpr std::uint8_t DirectoryAttribute=0x10;
inline constexpr int MaximumDirectoryDepth=256;
inline constexpr int MaximumFileContentLength=64 * 1024 * 1024;
inline constexpr std::string_view MagicText="MICROSOFT*XBOX*MEDIA";
inline constexpr int PointerUnit=4,AlignmentMask=3,NameDigits=4;
inline constexpr char Zero='0',Null='\0';
inline constexpr std::string_view DefaultNamePrefix="entry-";
}
