#pragma once
#include "../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Encoding {
RETROMEDIALAB_MEDIAENGINE_API std::size_t requiredByteCount(std::int32_t bitCount);
RETROMEDIALAB_MEDIAENGINE_API void packBits(Flux::BitView,MediaFileSystems::MutableBytes,bool paddingBit=false);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> packBits(Flux::BitView,bool paddingBit=false);
RETROMEDIALAB_MEDIAENGINE_API Flux::Bits unpackBits(MediaFileSystems::Bytes,std::int32_t bitCount);
}
