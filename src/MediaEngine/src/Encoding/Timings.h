#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Encoding {
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t bitCellTicks(std::string_view formatId);
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t indexTimeTicks(std::string_view formatId);
}
