#pragma once
#include "Request.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool canEncodeValidatedBytes(const SequentialEncodeRequest&,std::span<const std::string> formats,std::span<const std::string> machines);
}
