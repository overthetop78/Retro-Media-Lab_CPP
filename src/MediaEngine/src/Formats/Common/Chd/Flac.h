#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <span>
#include <vector>
#include <stop_token>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Chd {
class Flac final {
public:
    RETROMEDIALAB_MEDIAENGINE_API std::size_t decodeFrames(std::span<const std::uint8_t>,std::span<std::uint8_t>,bool bigEndian,std::stop_token={});
private:std::vector<std::int64_t> left_,right_;
};
}
