#pragma once
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Apple {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> sectorWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> nibbleWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> macintoshWriterAdapter();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> lisaWriterAdapter();
}
