#pragma once
#include "Models.h"
namespace RetroMediaLab::MediaEngine::Decoding {
struct ExplicitScore {
    std::int32_t validSectors=0,invalidPenalty=0,sectorsWithData=0;
    double confidence=0;
    std::int32_t structures=0;
};
RETROMEDIALAB_MEDIAENGINE_API double automaticScore(const Result&) noexcept;
RETROMEDIALAB_MEDIAENGINE_API ExplicitScore explicitScore(const Result&) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool betterAutomatic(const Result&,const Result&) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool betterExplicit(const Result&,const Result&) noexcept;
}
