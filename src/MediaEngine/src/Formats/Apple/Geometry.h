#pragma once
#include "GeometryConstants.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <span>

namespace RetroMediaLab::MediaEngine::Apple::Geometry {
namespace MacintoshGcr {
struct ImageGeometry {std::int32_t heads,blockCount,capacity;};
RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectors(std::int32_t cylinder);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorAddress address(std::int32_t logicalBlock,std::int32_t heads);
RETROMEDIALAB_MEDIAENGINE_API bool isSupportedCapacity(std::int32_t capacity) noexcept;
RETROMEDIALAB_MEDIAENGINE_API ImageGeometry forHeads(std::int32_t heads);
}
namespace LisaFileWare {
RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectors(std::int32_t cylinder);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorAddress address(std::int32_t logicalBlock);
}
enum class Kind {AppleII525Dos32,AppleII525Dos33,AppleII525ProDos,LisaFileWare,MacintoshGcr400,MacintoshGcr800,ProDos800,MacintoshMfm1440};
struct Layout {std::string_view name;std::int32_t capacity;std::span<const Kind> geometries;};
RETROMEDIALAB_MEDIAENGINE_API std::span<const Layout> layouts() noexcept;
RETROMEDIALAB_MEDIAENGINE_API const Layout* findLayout(std::int32_t capacity) noexcept;
}
