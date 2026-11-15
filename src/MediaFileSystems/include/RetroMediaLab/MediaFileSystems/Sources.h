#pragma once
#include "Export.h"
#include "Diagnostics.h"
#include "VolumeConstants.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaFileSystems {
using Bytes = std::span<const std::uint8_t>;
using MutableBytes = std::span<std::uint8_t>;
// Implementations own the backing source; reads are exact or throw a coded MediaError.
class RandomAccessData {
public:
    virtual ~RandomAccessData() = default;
    virtual std::uint64_t length() const noexcept = 0;
    virtual void readExactly(std::uint64_t offset, MutableBytes, std::stop_token = {}) const = 0;
};
class BlockMedia final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API BlockMedia(std::shared_ptr<const RandomAccessData>, std::uint32_t blockSize = VolumeConstants::DefaultBlockSize, bool hardDisk = false, std::optional<std::uint64_t> declaredBlockCount = {});
    std::uint64_t capacity() const noexcept { return source_->length(); }
    std::uint32_t blockSize() const noexcept { return blockSize_; }
    std::uint64_t blockCount() const noexcept { return declaredBlockCount_.value_or(capacity() / blockSize_); }
    bool isHardDisk() const noexcept { return hardDisk_; }
    RETROMEDIALAB_MEDIAFILESYSTEMS_API void readExactly(std::uint64_t offset, MutableBytes, std::stop_token = {}) const;
private:
    std::shared_ptr<const RandomAccessData> source_;
    std::uint32_t blockSize_;
    bool hardDisk_;
    std::optional<std::uint64_t> declaredBlockCount_;
};
}
