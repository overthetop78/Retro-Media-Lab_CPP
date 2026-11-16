#pragma once
#include "../MediaDocument.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> attachOpticalCompanionAudio(std::shared_ptr<const MediaDocument>,std::stop_token={});
}
