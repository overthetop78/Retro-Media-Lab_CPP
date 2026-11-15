#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool matchesSectorGeometry(const MediaFileSystems::SectorImage&,const MediaFileSystems::SectorGeometry&,bool requireFormatId = true);
RETROMEDIALAB_MEDIAENGINE_API bool hasCompleteSectorPayload(const MediaFileSystems::SectorImage&,std::uint32_t blockSize);
RETROMEDIALAB_MEDIAENGINE_API void writeLinearSectorImage(const MediaFileSystems::SectorImage&,const std::filesystem::path&,const MediaFileSystems::SectorGeometry&,bool requireFormatId = true,std::stop_token = {});
}
