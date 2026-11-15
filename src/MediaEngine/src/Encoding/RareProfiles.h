#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <optional>
#include <string_view>

namespace RetroMediaLab::MediaEngine::Encoding {
struct RareProfile {
    std::string_view encoderId;
    std::uint32_t cellTicks, indexTicks;
};
RETROMEDIALAB_MEDIAENGINE_API std::optional<RareProfile> rareProfile(std::string_view formatId);
}
