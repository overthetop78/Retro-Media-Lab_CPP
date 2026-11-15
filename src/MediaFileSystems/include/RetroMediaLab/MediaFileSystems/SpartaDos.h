#pragma once
#include "SectorImage.h"
#include "FileSystemModels.h"
namespace RetroMediaLab::MediaFileSystems::SpartaDos {
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token={});
}
