#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Decoding {
RETROMEDIALAB_MEDIAENGINE_API double confidence(std::int32_t sectors,std::int32_t structures,std::int32_t weight,double divisor);
RETROMEDIALAB_MEDIAENGINE_API double standardConfidence(std::int32_t sectors,std::int32_t structures);
RETROMEDIALAB_MEDIAENGINE_API double validityConfidence(std::int32_t valid,std::int32_t detected,double validDivisor,double detectedDivisor);
}
