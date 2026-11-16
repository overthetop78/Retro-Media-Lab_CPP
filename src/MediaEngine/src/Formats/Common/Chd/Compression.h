#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <memory>
#include <span>
#include <stop_token>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Chd {
// One instance belongs to one serialized hunk decoder; native workspaces are reused.
class Compression final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Compression();
    RETROMEDIALAB_MEDIAENGINE_API ~Compression();
    RETROMEDIALAB_MEDIAENGINE_API void deflate(std::span<const std::uint8_t>,std::span<std::uint8_t>,std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API void lzma(std::span<const std::uint8_t>,std::span<std::uint8_t>,std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API void zstd(std::span<const std::uint8_t>,std::span<std::uint8_t>,std::stop_token={});
private:struct Impl;std::unique_ptr<Impl> impl_;
};
}
