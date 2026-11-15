#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include "../../Models/Flux/Flux.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Nib {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(std::span<const Flux::Bits>);
RETROMEDIALAB_MEDIAENGINE_API void write(std::span<const Flux::Bits>,const std::filesystem::path&,std::stop_token={});
}
