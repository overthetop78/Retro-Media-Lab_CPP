#pragma once
#include "../Encoding/SectorImageEncoder.h"
#include "../Formats/Scp/Scp.h"
namespace RetroMediaLab::MediaEngine::Visualization {
class SectorFluxVisualizer final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SectorFluxVisualizer();
    RETROMEDIALAB_MEDIAENGINE_API explicit SectorFluxVisualizer(Encoding::SectorImageEncoder);
    RETROMEDIALAB_MEDIAENGINE_API bool canVisualize(const MediaFileSystems::SectorImage&) const;
    RETROMEDIALAB_MEDIAENGINE_API Scp::Image create(const MediaFileSystems::SectorImage&,std::stop_token = {}) const;
private:
    Encoding::SectorImageEncoder encoder_;
};
}

