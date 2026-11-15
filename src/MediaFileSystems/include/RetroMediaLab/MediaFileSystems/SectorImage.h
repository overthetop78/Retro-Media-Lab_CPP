#pragma once
#include "Sources.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
enum class SectorAddressingKind { Physical, Logical };
struct SectorAddress {
    std::int64_t cylinder = 0, head = 0, number = 0;
    bool operator==(const SectorAddress&) const = default;
};
struct SectorGeometry {
    std::string formatId;
    std::uint32_t blockSize = VolumeConstants::DefaultBlockSize, cylinders = 0, heads = 0, sectorsPerTrack = 0;
    SectorAddressingKind addressing = SectorAddressingKind::Physical;
};
struct SectorBlock {
    std::uint64_t logicalBlock = 0;
    SectorAddress address;
    std::vector<std::uint8_t> data;
    std::optional<bool> integrityValid = true;
    std::int32_t revolution = 0;
    std::optional<std::vector<std::uint8_t>> tag;
    std::optional<std::uint8_t> formatCode, diagnosticCode;
    std::vector<std::uint8_t> recordedErrorCheckCode, recordedDataPrefix;
    std::optional<bool> headerIntegrityValid, dataIntegrityValid, deletedDataMark;
    std::optional<std::uint8_t> recordedSizeCode;
    std::optional<SectorAddress> recordedAddress;
    std::optional<std::int32_t> recordedTrackOrder;
};
struct SectorImageOptions {
    bool allowVariableBlockSize = false;
    std::optional<std::uint64_t> capacity, logicalBlockCount;
};
class SectorImage {
public:
    virtual ~SectorImage() = default;
    virtual const SectorGeometry& geometry() const noexcept = 0;
    virtual std::uint64_t blockCount() const noexcept = 0;
    virtual std::uint64_t capacity() const noexcept = 0;
    virtual bool allowsVariableBlockSize() const noexcept = 0;
    virtual bool containsBlock(std::uint64_t) const noexcept = 0;
    // Metadata only: obtaining an address must not read the sector payload.
    virtual std::optional<SectorAddress> blockAddress(std::uint64_t) const = 0;
    virtual std::shared_ptr<const SectorBlock> block(std::uint64_t, std::stop_token = {}) const = 0;
    // Returning false stops enumeration without building a collection proportional to a disk's size.
    virtual void visitAvailableBlocks(const std::function<bool(std::uint64_t)>&, std::stop_token = {}) const = 0;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::shared_ptr<const SectorBlock> requireSectorBlock(const SectorImage&, std::uint64_t, std::stop_token = {});
struct SectorWriteBlock { std::uint64_t logicalBlock; SectorAddress address; std::vector<std::uint8_t> data; };
struct SectorWritePlan {
    SectorGeometry geometry;
    std::vector<SectorWriteBlock> blocks;
    std::optional<std::uint64_t> capacity, logicalBlockCount;
};
}
