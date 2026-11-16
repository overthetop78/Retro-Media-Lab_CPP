#pragma once
#include <string_view>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
namespace RetroMediaLab::MediaEngine::Xdvdfs::Constants {
inline constexpr std::string_view FormatId="microsoft.xbox.xdvdfs",WriterId="optical-xbox-xdvdfs",Extension=".iso",Magic="MICROSOFT*XBOX*MEDIA",RootSectorKey="xdvdfs.rootDirectorySector",RootSizeKey="xdvdfs.rootDirectorySize",SectorSizeKey="xdvdfs.sectorSize",VolumeName="Xbox XDVDFS";
inline constexpr std::string_view FileSystemId=MediaFileSystems::FileSystemIds::Xdvdfs;
inline constexpr int SectorBytes=2048,DescriptorSector=32,MagicOffset=0,RootSectorOffset=0x14,RootSizeOffset=0x18,TrailerOffset=0x7ec;
inline constexpr char ExtensionSeparator='.';
}
