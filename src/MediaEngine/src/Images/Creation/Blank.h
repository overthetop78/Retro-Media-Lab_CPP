#pragma once
#include "../../Models/Sectors/SectorImages.h"
namespace RetroMediaLab::MediaEngine {
class BlankSectorImageFactory final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const MediaFileSystems::SectorImage> create(std::string_view formatId, std::stop_token = {});
};
}
