#pragma once
#include "../../../Models/Blocks/BlockImage.h"
namespace RetroMediaLab::MediaEngine::AcornWinchester {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> decodeDescriptor(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API std::optional<HardDiskGeometry> geometry(std::span<const std::uint8_t>,std::optional<std::int64_t> dataLength={});
RETROMEDIALAB_MEDIAENGINE_API bool supportsGeometry(const HardDiskGeometry&);
RETROMEDIALAB_MEDIAENGINE_API void writeGeometry(std::span<std::uint8_t>,const HardDiskGeometry&);
RETROMEDIALAB_MEDIAENGINE_API void updateGeometryIfChanged(std::span<std::uint8_t>,const HardDiskGeometry&,std::int64_t dataLength);
RETROMEDIALAB_MEDIAENGINE_API bool tryWritePhysicalParameters(std::span<const std::uint8_t> descriptor,std::int64_t dataLength,std::span<std::uint8_t> flags);
}
