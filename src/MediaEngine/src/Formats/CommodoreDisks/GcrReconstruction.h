#pragma once
#include "Images.h"
#include "../../Decoding/Models.h"
#include <map>
namespace RetroMediaLab::MediaEngine::CommodoreDisks::Internal {
class GcrReconstruction {
public:
    RETROMEDIALAB_MEDIAENGINE_API void add(Decoding::Result,std::int32_t revolution,std::stop_token = {});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> finish(std::optional<std::string_view> requested,bool physicalSecondHead,std::stop_token = {}) &&;
private:
    struct Candidate {std::int32_t track,sector;MediaFileSystems::SectorBlock block;};
    std::map<std::pair<std::int32_t,std::int32_t>,std::size_t> lookup_;
    std::vector<Candidate> candidates_;
    std::int32_t maxTrack_=0;
    bool extendedData_=false;
};
}
