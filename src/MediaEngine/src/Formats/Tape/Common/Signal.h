#pragma once
#include "Pcm.h"
#include "../../../Models/Sequential/TapeSignal.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine::TapeSignal {
using Parameters=TapePcm::Parameters;
struct UefResult {
    MediaTapeSignal signal;
    std::map<std::int64_t,int> sourcePositions;
};
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal pcm(const MediaDocument&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API UefResult fromUef(const SequentialImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> toUef(std::shared_ptr<const MediaSource>,const MediaTapeSignal&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::span<const std::string_view> writableExtensions();
RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal read(const std::filesystem::path&,std::optional<std::string> machine={},Parameters parameters={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const std::filesystem::path&,const MediaTapeSignal&,Parameters parameters={},std::stop_token={});
}
