#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::Scp {
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t diskType(const MediaFileSystems::SectorImage&);
}

