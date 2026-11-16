#pragma once
#include "DiskDefinitions.h"
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Definitions {
// Each adapter owns its definitions and cached layouts, including across deferred calls.
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageReader> rawReader(Catalogue);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> rawWriter(Catalogue);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageReader> rawReader();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> rawWriter();
}
