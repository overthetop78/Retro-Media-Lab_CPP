#pragma once
#include "Scp.h"
#include "../../Models/Flux/ProtectedTrack.h"
#include <map>
namespace RetroMediaLab::MediaEngine::Scp {
RETROMEDIALAB_MEDIAENGINE_API Flux::ProtectedTrackImage protectedImage(const Image&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Image fromProtected(const Flux::ProtectedTrackImage&,const std::map<std::string,std::string>& metadata,std::stop_token = {});
}
