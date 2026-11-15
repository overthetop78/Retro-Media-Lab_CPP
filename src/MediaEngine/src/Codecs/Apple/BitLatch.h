#pragma once
#include "../../Models/Flux/Flux.h"
namespace RetroMediaLab::MediaEngine::Apple {
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> readBytes(Flux::BitView,std::int64_t& offset,std::int64_t count,std::stop_token = {});
}
