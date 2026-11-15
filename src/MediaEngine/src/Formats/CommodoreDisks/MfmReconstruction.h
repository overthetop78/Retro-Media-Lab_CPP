#pragma once
#include "Images.h"
#include "../../Decoding/Models.h"
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine::CommodoreDisks::Internal {
class MfmReconstruction {
public:
    RETROMEDIALAB_MEDIAENGINE_API void add(Decoding::Result,std::int32_t cylinder,std::int32_t head,std::int32_t revolution,std::stop_token = {});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> finish(std::stop_token = {}) &&;
private:
    struct Candidate {std::int32_t cylinder,head,sector;MediaFileSystems::SectorBlock block;};
    std::map<std::tuple<std::int32_t,std::int32_t,std::int32_t>,std::size_t> lookup_;
    std::vector<Candidate> candidates_;
};
}
