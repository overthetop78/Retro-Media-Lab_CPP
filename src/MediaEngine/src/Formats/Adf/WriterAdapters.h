#pragma once
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Adf {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> acornWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> amigaWriterAdapter();
}
