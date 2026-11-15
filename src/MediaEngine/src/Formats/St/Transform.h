#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::St {
RETROMEDIALAB_MEDIAENGINE_API bool canTransformGeometry(const MediaFileSystems::SectorImage&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> transform(const MediaFileSystems::SectorImage&,std::string_view,bool completeMissingSectors=false,std::stop_token={});
}
