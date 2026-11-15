#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume exploreSectorImage(const MediaFileSystems::SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume exploreSectorSource(std::shared_ptr<const MediaFileSystems::RandomAccessData>,MediaFileSystems::SectorGeometry,std::uint64_t offset = 0,std::stop_token = {});
}
