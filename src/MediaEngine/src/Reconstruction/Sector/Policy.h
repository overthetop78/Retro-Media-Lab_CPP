#pragma once
#include "../../Encoding/Track.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <span>

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
using Image = MediaFileSystems::SectorImage;
using Block = MediaFileSystems::SectorBlock;
using Address = MediaFileSystems::SectorAddress;
struct Item { std::shared_ptr<const Block> block; Address address; };
RETROMEDIALAB_MEDIAENGINE_API std::int32_t checkedNumber(std::int64_t);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Block> availableBlock(const Image&, std::uint64_t, std::stop_token = {});
class Policy {
public:
    virtual ~Policy() = default;
    virtual bool canHandle(const Image&) const = 0;
    virtual std::string_view encoderId(const Image&) const = 0;
    virtual Address visualAddress(const Image&, Address address) const { return address; }
    RETROMEDIALAB_MEDIAENGINE_API virtual std::vector<Encoding::Sector> createTrackSectors(const Image&, std::span<const Item>, std::stop_token = {}) const;
    virtual Encoding::Attributes trackAttributes(const Image&, std::int32_t) const { return {}; }
    RETROMEDIALAB_MEDIAENGINE_API virtual std::uint32_t bitCellTicks(const Image&, std::int32_t) const;
    RETROMEDIALAB_MEDIAENGINE_API virtual std::uint32_t indexTimeTicks(const Image&, std::int32_t) const;
};
}

