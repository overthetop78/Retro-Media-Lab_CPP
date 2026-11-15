#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::RotatingChecksum {
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t compute(MediaFileSystems::Bytes,std::stop_token = {});
}
