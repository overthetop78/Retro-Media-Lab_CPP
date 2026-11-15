#pragma once
#include <RetroMediaLab/MediaFileSystems/AmigaFlatArchive.h>
#include "Constants.h"
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive::Internal {
struct Descriptor {std::string name;std::uint32_t length;};
struct ReadResult {std::vector<std::uint8_t> bytes;std::vector<std::uint64_t> missing,invalid;};
ReadResult data(const SectorImage&,std::uint64_t offset,std::uint64_t length,std::stop_token);
std::optional<std::vector<Descriptor>> directory(const SectorImage&,std::stop_token);
}
