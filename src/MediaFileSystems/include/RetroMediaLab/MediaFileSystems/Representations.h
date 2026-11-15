#pragma once
#include "SectorImage.h"
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace RetroMediaLab::MediaFileSystems {
class ImageRepresentation {
public:
    virtual ~ImageRepresentation() = default;
    virtual std::optional<std::int64_t> logicalLength() const = 0;
    virtual bool supportsRandomAccess() const = 0;
    virtual bool supportsSequentialAccess() const = 0;
};
class BlockRepresentation : public ImageRepresentation {
public:
    virtual std::int64_t capacity() const = 0;
    virtual std::int32_t logicalBlockSize() const = 0;
    virtual std::int64_t logicalBlockCount() const = 0;
    // The caller keeps destination alive until the returned operation completes.
    virtual std::future<void> readExactlyAsync(std::int64_t address, MutableBytes destination, std::stop_token = {}) const = 0;
};
class SectorRepresentation : public ImageRepresentation {
public:
    virtual const SectorImage& image() const = 0;
};
class OpticalTrack {
public:
    virtual ~OpticalTrack() = default;
    virtual std::int32_t sessionNumber() const = 0;
    virtual std::int32_t trackNumber() const = 0;
    virtual std::int64_t firstSector() const = 0;
    virtual std::int64_t sectorCount() const = 0;
    virtual std::int32_t userDataLength() const = 0;
    virtual bool isAudio() const = 0;
    virtual std::future<std::vector<std::uint8_t>> readUserDataAsync(std::int64_t relativeSector, std::stop_token = {}) const = 0;
};
class OpticalRepresentation : public ImageRepresentation {
public:
    // nullopt preserves the distinction between absent and empty track lists.
    virtual std::optional<std::span<const std::shared_ptr<const OpticalTrack>>> tracks() const = 0;
};
}
