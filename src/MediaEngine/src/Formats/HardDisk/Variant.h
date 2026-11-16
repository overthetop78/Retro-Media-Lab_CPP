#pragma once
#include "../../Images/MediaDocument.h"
#include "../../Models/Blocks/BlockImage.h"
namespace RetroMediaLab::MediaEngine::HardDiskContainers {
RETROMEDIALAB_MEDIAENGINE_API bool selectFixedVariant(const MediaDocument&,const BlockImage&,std::string_view format);
}
