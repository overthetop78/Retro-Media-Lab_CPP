#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Fat12Reinterpretation {
RETROMEDIALAB_MEDIAENGINE_API std::optional<MediaFileSystems::SectorGeometry> targetGeometry(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorGeometry validate(const MediaFileSystems::SectorImage&,std::string_view target,bool sourceIsHybrid=false,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void convert(std::shared_ptr<const MediaFileSystems::SectorImage>,const std::filesystem::path&,std::string target,bool sourceIsHybrid=false,std::stop_token={});
}
