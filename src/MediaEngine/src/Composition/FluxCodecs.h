#pragma once
#include "../Decoding/Decoder.h"
#include "../Encoding/Track.h"
namespace RetroMediaLab::MediaEngine::Composition {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const Decoding::Decoder>> defaultDecoders();
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const Encoding::TrackEncoder>> defaultEncoders();
}

