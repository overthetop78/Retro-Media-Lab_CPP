#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <optional>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> trySectorSizeCode(std::int64_t) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t sectorSizeCode(std::int64_t);
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t sectorSizeBytes(std::uint8_t);
}
