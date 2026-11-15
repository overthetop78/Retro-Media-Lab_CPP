#pragma once
#include "Constants.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <RetroMediaLab/MediaEngine/Export.h>
#include <vector>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::AppleRwts18 {
struct DecodedPayload {std::vector<std::uint8_t> data;bool valid=false;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodePayload(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API DecodedPayload decodePayload(MediaFileSystems::Bytes,std::stop_token = {});
}
