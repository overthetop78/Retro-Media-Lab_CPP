#pragma once
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::Atari8Bit {
RETROMEDIALAB_MEDIAENGINE_API std::string formatId(std::int32_t sectorSize,std::int32_t sectorCount);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorGeometry geometry(std::int32_t sectorSize,std::int32_t sectorCount);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Constants::Profile> profile(std::string_view id);
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t payloadLength(const Constants::Profile&) noexcept;
}
