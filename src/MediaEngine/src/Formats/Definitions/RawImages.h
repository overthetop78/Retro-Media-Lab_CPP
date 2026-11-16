#pragma once
#include "DiskDefinitions.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include "../../Images/Readers.h"
#include "../../Images/Writers.h"
namespace RetroMediaLab::MediaEngine::Definitions {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> createImage(const DiskDefinition&,std::span<const Sector>,std::vector<MediaFileSystems::SectorBlock>,std::stop_token={});
// Each adapter owns its definitions and cached layouts, including across deferred calls.
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageReader> rawReader(Catalogue);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> rawWriter(Catalogue);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageReader> rawReader();
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> rawWriter();
}
