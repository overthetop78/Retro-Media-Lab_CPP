#pragma once
#include "SectorImage.h"
#include "FileSystemModels.h"
namespace RetroMediaLab::MediaFileSystems::Ucsd {
enum class ByteOrder {LittleEndian,BigEndian};
enum class FileKind {Untyped,ExternalDisk,Code,Text,Info,Data,Graphics,Photo,SecureDirectory};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view kindName(FileKind) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token={});
}
