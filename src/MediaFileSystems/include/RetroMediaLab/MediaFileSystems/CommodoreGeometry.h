#pragma once
#include "Export.h"
#include "CommodoreGeometryConstants.h"
#include <utility>
namespace RetroMediaLab::MediaFileSystems::Commodore {
struct Address {std::int32_t track=0,sector=0,side=0;bool operator==(const Address&) const=default;};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t sectors1541(std::int32_t track);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t blocksPerSide1541(std::int32_t tracks);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t sideBlock1541(std::int32_t track,std::int32_t sector,std::int32_t tracks);
RETROMEDIALAB_MEDIAFILESYSTEMS_API Address address1541(std::int32_t block,std::int32_t tracksPerSide,std::int32_t sides);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t cylinder1541(std::int32_t track) noexcept;
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t block1571(std::int32_t track,std::int32_t sector,std::int32_t tracksPerSide,std::int32_t side);
RETROMEDIALAB_MEDIAFILESYSTEMS_API Address address1571(std::int32_t block,std::int32_t tracksPerSide);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t block1581(std::int32_t track,std::int32_t sector);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::pair<std::int32_t,std::int32_t> address1581(std::int32_t block);
// Preserves the source's unchecked arithmetic; callers validate physical coordinates where required.
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::int32_t physicalBlock1581(std::int32_t cylinder,std::int32_t head,std::int32_t sector) noexcept;
}
