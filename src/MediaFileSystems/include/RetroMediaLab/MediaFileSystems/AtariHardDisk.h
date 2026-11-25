#pragma once

#include "Volumes.h"
#include <string_view>
#include <vector>

namespace RetroMediaLab::MediaFileSystems::AtariHardDisk {
inline constexpr std::uint32_t PhysicalSectorSize = 512;
enum class Layout { Ahdi, IdeWordSwapped };
inline constexpr std::string_view AhdiTableId = "ahdi", IdeTableId = "ahdi-ide", Fat16Id = "fat16";

inline void swapWords(MutableBytes bytes) {
    for (std::size_t i = 1; i < bytes.size(); i += 2) std::swap(bytes[i - 1], bytes[i]);
}
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<Layout> probe(Bytes, std::uint64_t capacity);
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<VolumeDetectionResult> detectVolumes(
    const BlockMedia&, std::stop_token = {});

// Returns the initialized prefix of a disk with one AHDI partition, or no value for an unsupported size.
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::vector<std::uint8_t>> prepare(
    std::uint64_t capacity, Layout, std::stop_token = {});
}
