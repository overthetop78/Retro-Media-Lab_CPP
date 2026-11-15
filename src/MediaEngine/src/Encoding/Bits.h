#pragma once
#include "../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Encoding {
RETROMEDIALAB_MEDIAENGINE_API void raw(Flux::Bits&,MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API void rawHex(Flux::Bits&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API void rawBits(Flux::Bits&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API void mfm(Flux::Bits&,MediaFileSystems::Bytes,bool previousData = false);
RETROMEDIALAB_MEDIAENGINE_API void fm(Flux::Bits&,MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API void doubleFm(Flux::Bits&,MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API void gap(Flux::Bits&,std::int64_t count,bool allOnes = false);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> compactMfm(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> compactFm(MediaFileSystems::Bytes);
}
