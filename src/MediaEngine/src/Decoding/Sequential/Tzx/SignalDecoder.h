#pragma once
#include "../Models.h"
namespace RetroMediaLab::MediaEngine {
class SequentialSignalDecodeResult final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialSignalDecodeResult(std::shared_ptr<const SequentialImage>,std::vector<SequentialSegment>,std::vector<MediaFileSystems::MessageRecipe>);
    const SequentialImage& timeline()const noexcept{return *timeline_;}
    std::span<const SequentialSegment> sourceSegments()const noexcept{return source_;}
    std::span<const MediaFileSystems::MessageRecipe> messages()const noexcept{return messages_;}
private:
    std::shared_ptr<const SequentialImage> timeline_;std::vector<SequentialSegment> source_;std::vector<MediaFileSystems::MessageRecipe> messages_;
};
class TzxSignalDecoder final {
public:
    RETROMEDIALAB_MEDIAENGINE_API std::future<SequentialSignalDecodeResult> decodeAsync(std::shared_ptr<const MediaDocument>,std::stop_token={})const;
};
}
