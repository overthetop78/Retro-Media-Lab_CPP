#pragma once

#include "Sources.h"
#include <optional>
#include <string>
#include <vector>

namespace RetroMediaLab::MediaFileSystems::Rdb {
inline constexpr std::uint32_t SectorSize = 512;
inline constexpr std::uint32_t HeaderSearchSectors = 16;
inline constexpr std::uint32_t DefaultHeads = 1;
inline constexpr std::uint32_t DefaultSectorsPerTrack = 32;
inline constexpr std::uint32_t DosOfs = 0x444f5300;
inline constexpr std::uint32_t DosFfs = 0x444f5301;

struct Partition {
    std::uint64_t offset, length;
    std::string name;
    std::uint32_t dosType;
    bool active;
};

// Invalid layouts have no table. Returned bytes cover only the initialized RDSK and PART blocks.
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<std::vector<std::uint8_t>> prepare(
    std::uint64_t capacity, std::span<const Partition>, std::uint32_t heads = DefaultHeads,
    std::uint32_t sectorsPerTrack = DefaultSectorsPerTrack, std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validHeader(Bytes);
}
