#pragma once
#include "Writer.h"
#include "../../Encoding/SectorImageEncoder.h"
namespace RetroMediaLab::MediaEngine::Scp {
class SectorConverter final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SectorConverter();
    RETROMEDIALAB_MEDIAENGINE_API explicit SectorConverter(Encoding::SectorImageEncoder);
    RETROMEDIALAB_MEDIAENGINE_API bool canCreate(const MediaFileSystems::SectorImage&) const;
    RETROMEDIALAB_MEDIAENGINE_API Image create(const MediaFileSystems::SectorImage&,std::stop_token = {}) const;
    RETROMEDIALAB_MEDIAENGINE_API void convert(const MediaFileSystems::SectorImage&,const ImageOutput&,std::stop_token = {}) const;
    RETROMEDIALAB_MEDIAENGINE_API void convert(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token = {}) const;
private:
    Encoding::SectorImageEncoder encoder_;
};
}

