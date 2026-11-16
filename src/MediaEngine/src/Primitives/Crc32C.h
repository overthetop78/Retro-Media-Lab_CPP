#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <span>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Primitives {
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t crc32c(std::span<const std::uint8_t>)noexcept;
}
