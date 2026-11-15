#pragma once
#include "SectorImage.h"
#include "CpmLayoutConstants.h"
#include "FileSystemModels.h"
#include <span>
namespace RetroMediaLab::MediaFileSystems::Cpm {
struct AmstradDiskSpecification {std::uint8_t tracks,sectorsPerTrack;std::int32_t sectorSize;std::uint8_t reservedTracks;std::int32_t allocationSize;std::uint8_t directoryBlocks;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<AmstradDiskSpecification> parseAmstradSpecification(Bytes);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::string_view> catalogFormatIds() noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::string_view> amstradCatalogFormatIds() noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool looksLikeCpcSystemDirectory(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canReadAmstrad(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume readAmstrad(const SectorImage&,std::stop_token = {});
}
