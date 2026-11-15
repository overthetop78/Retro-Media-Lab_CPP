#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::St {
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
}
