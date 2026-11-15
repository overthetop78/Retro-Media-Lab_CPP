#pragma once
#include "SectorImage.h"
#include "FileSystemModels.h"
namespace RetroMediaLab::MediaFileSystems::Rt11 {
enum class EntryStatus:std::uint16_t {Tentative=0x0100,Empty=0x0200,Permanent=0x0400,EndOfSegment=0x0800,Protected=0x8000};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string radix50(std::uint16_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> decodeDate(std::uint16_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeAscii(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool looksLike(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token={});
}
