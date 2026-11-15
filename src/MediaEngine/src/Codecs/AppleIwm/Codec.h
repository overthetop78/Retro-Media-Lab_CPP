#pragma once
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <array>
#include <vector>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::AppleIwm {
struct DecodedPayload {std::vector<std::uint8_t> data;std::array<std::uint8_t,Constants::ChecksumSymbolCount> checksum;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encode(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API DecodedPayload decode(MediaFileSystems::Bytes,std::stop_token = {});
}
