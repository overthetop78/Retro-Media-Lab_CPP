#pragma once
#include "MediaDocument.h"
#include <RetroMediaLab/MediaEngine/MediaExplorationProgress.h>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API void reportWriteInput(const MediaDocument&,const std::string& target,const MediaExplorationProgressCallback&,std::stop_token={});
}
