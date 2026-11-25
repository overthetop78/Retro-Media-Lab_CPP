#pragma once
#include "Sources.h"
#include "MessageRecipe.h"
#include <optional>
#include <string>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
class VolumeDescriptor final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API VolumeDescriptor(std::int64_t start, std::int64_t length, std::string origin,
        std::optional<std::string> partitionTable = {}, std::optional<std::int32_t> partitionNumber = {},
        std::optional<std::int32_t> sessionNumber = {}, std::optional<std::int32_t> trackNumber = {},
        std::optional<std::string> fileSystemId = {}, std::optional<std::string> partitionType = {},
        std::optional<std::string> partitionId = {}, std::optional<std::string> name = {},std::optional<MessageRecipe> nameRecipe = {});
    std::uint64_t start() const noexcept { return std::uint64_t(start_); }
    std::uint64_t length() const noexcept { return std::uint64_t(length_); }
    const std::string& origin() const noexcept { return origin_; }
    const std::optional<std::string>& partitionTable() const noexcept { return partitionTable_; }
    const std::optional<std::int32_t>& partitionNumber() const noexcept { return partitionNumber_; }
    const std::optional<std::int32_t>& sessionNumber() const noexcept { return sessionNumber_; }
    const std::optional<std::int32_t>& trackNumber() const noexcept { return trackNumber_; }
    const std::optional<std::string>& fileSystemId() const noexcept { return fileSystemId_; }
    const std::optional<std::string>& partitionType() const noexcept { return partitionType_; }
    const std::optional<std::string>& partitionId() const noexcept { return partitionId_; }
    const std::optional<std::string>& name() const noexcept { return name_; }
    const std::optional<MessageRecipe>& nameRecipe() const noexcept { return nameRecipe_; }
    bool operator==(const VolumeDescriptor&) const = default;
private:
    std::int64_t start_, length_;
    std::string origin_;
    std::optional<std::string> partitionTable_;
    std::optional<std::int32_t> partitionNumber_, sessionNumber_, trackNumber_;
    std::optional<std::string> fileSystemId_, partitionType_, partitionId_, name_;
    std::optional<MessageRecipe> nameRecipe_;
};
struct VolumeDetectionResult {
    std::vector<VolumeDescriptor> volumes;
    std::vector<Diagnostic> diagnostics;
};
// nullopt means the detector declined this representation; an empty result can carry errors.
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<VolumeDetectionResult> detectMbr(const BlockMedia&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<VolumeDetectionResult> detectGpt(const BlockMedia&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<VolumeDetectionResult> detectRdb(const BlockMedia&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API VolumeDetectionResult detectWholeMedia(const BlockMedia&, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API VolumeDetectionResult detectBlockVolumes(const BlockMedia&, std::stop_token = {});
}
