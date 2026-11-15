#pragma once
#include "FmAddressMarkConstants.h"
#include <RetroMediaLab/MediaEngine/Export.h>
namespace RetroMediaLab::MediaEngine::FmAddressMarks {
using Pattern=std::array<std::uint8_t,Constants::ByteCount>;
RETROMEDIALAB_MEDIAENGINE_API const Pattern& forMark(std::uint8_t);
}
