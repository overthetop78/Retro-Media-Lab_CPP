#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::Adfs {
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool hasTimestamp(std::uint32_t load) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Timestamp> decodeTime(std::uint32_t load,std::uint32_t execute);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string describe(std::uint32_t load,std::uint32_t execute);
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const BlockMedia&,std::uint64_t start,std::uint64_t length,std::string_view formatId = {},std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const BlockMedia&,std::uint64_t start,std::uint64_t length,std::string_view formatId = {},std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::uint8_t> createBbcHeader(std::uint32_t sectorCount);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<std::uint8_t> createDHeader();
}
