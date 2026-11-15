#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Epson {
RETROMEDIALAB_MEDIAENGINE_API void writeRaw(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view target,std::stop_token={});
}
