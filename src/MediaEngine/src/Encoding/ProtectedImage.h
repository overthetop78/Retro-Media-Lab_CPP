#pragma once
#include "Track.h"
#include "../Models/Flux/ProtectedTrack.h"
namespace RetroMediaLab::MediaEngine::Encoding {
RETROMEDIALAB_MEDIAENGINE_API Flux::ProtectedTrackImage protectedImage(std::span<const DiskTrack>,std::int32_t resolutionNanoseconds,std::stop_token = {});
}
