#pragma once
#include "Signal.h"
#include "../../../Decoding/Sequential/Models.h"
namespace RetroMediaLab::MediaEngine::TapeSignal {
RETROMEDIALAB_MEDIAENGINE_API int carrierStart(const MediaTapeSignal&,int position);
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal associateBlocks(const MediaTapeSignal&,const SequentialDecodeResult&,const SequentialImage&,const std::map<std::int64_t,int>& positions,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> indexBlocks(std::shared_ptr<const MediaDocument>,std::stop_token={});
}
