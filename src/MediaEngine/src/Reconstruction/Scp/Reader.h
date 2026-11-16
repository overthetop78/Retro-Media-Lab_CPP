#pragma once
#include "Candidates.h"
#include <RetroMediaLab/MediaFileSystems/SectorRegistry.h>
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
class Reader final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Reader(Candidates,MediaFileSystems::SectorRegistry={});
    RETROMEDIALAB_MEDIAENGINE_API Image read(const std::filesystem::path&,std::optional<std::string> format={},const Progress& = {},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API Image read(const Flux::ProtectedTrackImage&,std::optional<std::string> format={},const Progress& = {},std::stop_token={})const;
private:
    Image readDefaults(const std::function<Image(const Candidate&)>&,bool fluxOnly,std::stop_token)const;
    Candidates candidates_;
    MediaFileSystems::SectorRegistry fileSystems_;
};
}
