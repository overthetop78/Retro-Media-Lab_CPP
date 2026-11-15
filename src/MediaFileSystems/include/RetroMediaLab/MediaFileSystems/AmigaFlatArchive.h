#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive {
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token = {});
}
