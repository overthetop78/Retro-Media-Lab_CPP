#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::Fat {
struct BpbGeometry { std::uint32_t sectorSize, totalSectors, cylinders, heads, sectorsPerTrack; };
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<BpbGeometry> detectBpbGeometry(Bytes boot, std::optional<std::uint64_t> imageLength = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> decodeDateTime(std::uint16_t date, std::uint16_t time);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeFixed(Bytes, std::size_t offset, std::size_t length);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::string> volumeLabel(Bytes root);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string bootVolumeLabel(Bytes boot);
}
