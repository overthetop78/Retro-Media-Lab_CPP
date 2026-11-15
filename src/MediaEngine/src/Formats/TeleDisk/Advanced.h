#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::TeleDisk {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> decompressAdvanced(MediaFileSystems::Bytes,std::stop_token={});
}
