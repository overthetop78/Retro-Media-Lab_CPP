#pragma once
#include "Records.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::TeleDisk {
RETROMEDIALAB_MEDIAENGINE_API bool hasRepresentablePayload(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const Image&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
}
