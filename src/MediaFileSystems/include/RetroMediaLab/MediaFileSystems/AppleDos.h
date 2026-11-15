#pragma once
#include "Migration.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::AppleDos {
struct BinaryContent {std::vector<std::uint8_t> content;std::uint16_t loadAddress;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validName(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::uint8_t> volumeNumber(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string volumeName(std::uint8_t);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string decodeName(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::uint8_t> encodeBinary(Bytes,std::uint16_t loadAddress);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<BinaryContent> decodeBinary(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&,std::string_view formatId,std::stop_token = {});
}
