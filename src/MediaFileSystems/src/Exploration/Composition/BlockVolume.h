#pragma once
#include <RetroMediaLab/MediaFileSystems/AtariHardDisk.h>
#include <RetroMediaLab/MediaFileSystems/SectorReader.h>

namespace RetroMediaLab::MediaFileSystems::Composition {
class BlockVolume final : public SectorImage {
    const BlockRepresentation& blocks_;
    std::uint64_t start_, length_;
    SectorGeometry geometry_;
    bool wordSwapped_;
public:
    BlockVolume(const BlockRepresentation& blocks, const VolumeDescriptor& volume, std::string_view format,
        std::uint32_t blockSize, bool wordSwapped = false)
        : blocks_(blocks), start_(volume.start()), length_(volume.length()),
          geometry_{std::string(format), blockSize, 0, 0, 0, SectorAddressingKind::Logical},
          wordSwapped_(wordSwapped) {}

    const SectorGeometry& geometry() const noexcept override { return geometry_; }
    std::uint64_t blockCount() const noexcept override { return length_ / geometry_.blockSize; }
    std::uint64_t capacity() const noexcept override { return length_; }
    bool allowsVariableBlockSize() const noexcept override { return false; }
    bool containsBlock(std::uint64_t index) const noexcept override { return index < blockCount(); }
    std::optional<SectorAddress> blockAddress(std::uint64_t index) const override {
        if (!containsBlock(index)) return {};
        return SectorAddress{0, 0, std::int64_t(index)};
    }
    std::shared_ptr<const SectorBlock> block(std::uint64_t index, std::stop_token token) const override {
        const auto address = blockAddress(index);
        if (!address) return {};
        SectorBlock sector;
        sector.logicalBlock = index;
        sector.address = *address;
        sector.data.resize(geometry_.blockSize);
        blocks_.readExactlyAsync(std::int64_t(start_ + index * geometry_.blockSize), sector.data, token).get();
        if (wordSwapped_) AtariHardDisk::swapWords(sector.data);
        return std::make_shared<SectorBlock>(std::move(sector));
    }
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visit, std::stop_token token) const override {
        for (std::uint64_t index = 0; index < blockCount(); ++index) {
            checkCancellation(token);
            if (!visit(index)) break;
        }
    }
};

}
