#pragma once
#include <exception>
#include <RetroMediaLab/MediaEngine/Export.h>
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API bool isImageReadRejection(const std::exception_ptr&);
RETROMEDIALAB_MEDIAENGINE_API bool isUnsupportedImageRead(const std::exception_ptr&);
}
