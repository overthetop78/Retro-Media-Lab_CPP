#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Apple {
RETROMEDIALAB_MEDIAENGINE_API std::int32_t proDosToPhysicalSector(std::int32_t);
RETROMEDIALAB_MEDIAENGINE_API std::int32_t physicalToDosFileSector(std::int32_t);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> dosToProDos(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> proDosToDos(MediaFileSystems::Bytes);
}
