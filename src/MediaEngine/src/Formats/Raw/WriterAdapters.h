#pragma once
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Raw {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> ibmWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> msxWriterAdapter();
}
