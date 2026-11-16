#pragma once
#include "Pcm.h"
#include "../../../Models/Sequential/TapeSignal.h"
namespace RetroMediaLab::MediaEngine::TapeSignal {
struct UefResult {
    MediaTapeSignal signal;
    std::map<std::int64_t,int> sourcePositions;
};
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal pcm(const MediaDocument&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API UefResult fromUef(const SequentialImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> toUef(std::shared_ptr<const MediaSource>,const MediaTapeSignal&,std::stop_token={});
}
