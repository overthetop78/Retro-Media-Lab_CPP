#pragma once
#include "SectorImages.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> tryUniformSectorImage(MediaFileSystems::SectorGeometry,const std::vector<MediaFileSystems::SectorBlock>&,std::stop_token = {});
}
