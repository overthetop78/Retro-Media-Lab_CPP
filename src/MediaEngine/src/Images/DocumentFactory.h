#pragma once
#include "MediaDocument.h"
#include "../Models/Flux/ProtectedTrack.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> floppySectorDocument(std::shared_ptr<const MediaSource>,std::shared_ptr<const MediaFileSystems::SectorImage>,std::map<std::string,std::string> metadata={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> floppyFluxDocument(std::shared_ptr<const MediaSource>,std::string formatId,std::shared_ptr<const Flux::ProtectedTrackImage>,std::map<std::string,std::string> metadata={});
}
