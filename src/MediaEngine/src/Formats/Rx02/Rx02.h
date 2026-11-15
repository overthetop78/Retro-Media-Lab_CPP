#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Rx02 {
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorGeometry geometry();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaFileSystems::SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
}
