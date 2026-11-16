#pragma once
#include "../../Models/Flux/ProtectedTrack.h"
#include <map>
#include <string>
namespace RetroMediaLab::MediaEngine::KryoFlux {
using Metadata=std::map<std::string,std::string>;
struct DecodedTrack {std::vector<Flux::TrackFluxRevolution> revolutions;Metadata metadata;};
RETROMEDIALAB_MEDIAENGINE_API bool containsInfo(std::span<const std::uint8_t>);
RETROMEDIALAB_MEDIAENGINE_API DecodedTrack readTrack(std::span<const std::uint8_t>,std::stop_token={});
}
