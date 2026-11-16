#pragma once
#include "Models.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool canDecodeTape(const MediaDocument&,const std::optional<std::string>& machine,std::span<const std::string> formats,std::span<const std::string> machines);
}
