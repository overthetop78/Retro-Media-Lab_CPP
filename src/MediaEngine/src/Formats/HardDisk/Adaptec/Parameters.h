#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <span>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Adaptec {
RETROMEDIALAB_MEDIAENGINE_API void writeInitialParameters(std::span<std::uint8_t>,std::int32_t sectorSize,std::int32_t cylinders,std::int32_t heads,std::uint16_t reducedWriteCurrent=0,std::uint8_t landingZone=0,std::uint8_t stepCode=0);
}
