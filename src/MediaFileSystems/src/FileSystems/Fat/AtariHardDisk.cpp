#include <RetroMediaLab/MediaFileSystems/AtariHardDisk.h>
#include "../../Primitives/Binary.h"
#include <RetroMediaLab/MediaFileSystems/VolumeConstants.h>
#include <set>

namespace RetroMediaLab::MediaFileSystems::AtariHardDisk {
namespace {
constexpr std::uint64_t MiB = 1024 * 1024;
constexpr std::uint64_t MinimumVolume = 8 * MiB, MaximumVolume = 1024 * MiB, BigPartition = 32 * MiB;
constexpr auto MaxBpbSectors = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint32_t ClusterSectors = 2, ReservedSectors = 1, FatCount = 2;
constexpr std::uint32_t RootEntries = 512, EntryBytes = 32, FatEntryBytes = sizeof(std::uint16_t);
constexpr std::uint32_t FirstPartitionSector = 1, SectorsPerTrack = 32, Heads = 1;
constexpr std::uint16_t MediaEntry = 0xfff8, EndOfChain = 0xffff;
constexpr std::uint8_t PartitionPresent = 1, FixedMedia = 0xf8;
constexpr std::array<std::uint8_t, 2> BootBranch{0x60, 0x1c};
constexpr std::string_view BootName = "RetroMediaLab ", GemPartition = "GEM", BigGemPartition = "BGM";
constexpr std::size_t DiskSize = 0x1c2, PartitionFlags = 0x1c6, PartitionType = 0x1c7;
constexpr std::size_t PartitionStart = 0x1ca, PartitionSize = 0x1ce;
constexpr std::size_t PartitionCount = 4, PartitionEntrySize = 12, PartitionTypeSize = 3;
constexpr std::string_view ExtendedPartition = "XGM";

struct Partition {
    std::string type;
    std::uint64_t start, count;
};

std::optional<Partition> partition(Bytes root, std::size_t index) {
    const auto offset = index * PartitionEntrySize;
    if (!(root[PartitionFlags + offset] & PartitionPresent)) return {};
    const auto type = root.subspan(PartitionType + offset, PartitionTypeSize);
    if (!std::all_of(type.begin(), type.end(), [](auto c) {
        return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    })) return {};
    return Partition{std::string(type.begin(), type.end()), Binary::big<std::uint32_t>(root, PartitionStart + offset),
        Binary::big<std::uint32_t>(root, PartitionSize + offset)};
}

bool fits(const Partition& entry, std::uint64_t limit) {
    return entry.start > 0 && entry.start < limit && entry.count > 0 && entry.count <= limit - entry.start;
}

std::uint64_t diskSectors(Bytes root, std::uint64_t capacity) {
    const auto declared = Binary::big<std::uint32_t>(root, DiskSize);
    return declared <= capacity / PhysicalSectorSize ? declared : 0;
}

bool validRoot(Bytes root, std::uint64_t capacity) {
    const auto sectors = diskSectors(root, capacity);
    if (!sectors) return false;
    for (std::size_t i = 0; i < PartitionCount; ++i) {
        if (const auto entry = partition(root, i); entry && fits(*entry, sectors)) return true;
    }
    return false;
}
constexpr std::size_t BootNameOffset = 2, BpbSectorBytes = 11, BpbCluster = 13, BpbReserved = 14;
constexpr std::size_t BpbFatCount = 16, BpbRootEntries = 17, BpbTotal = 19, BpbMedia = 21;
constexpr std::size_t BpbFatSectors = 22, BpbTrackSectors = 24, BpbHeads = 26;

void setBpb(MutableBytes boot, std::size_t offset, std::uint32_t value) {
    Binary::writeLittle(boot, offset, std::uint16_t(value));
}
}

std::optional<Layout> probe(Bytes root, std::uint64_t capacity) {
    if (root.size() < PhysicalSectorSize || capacity % PhysicalSectorSize) return {};
    root = root.first(PhysicalSectorSize);
    if (validRoot(root, capacity)) return Layout::Ahdi;
    std::array<std::uint8_t, PhysicalSectorSize> swapped;
    std::copy(root.begin(), root.end(), swapped.begin());
    swapWords(swapped);
    if (validRoot(swapped, capacity)) return Layout::IdeWordSwapped;
    return {};
}

std::optional<VolumeDetectionResult> detectVolumes(const BlockMedia& media, std::stop_token token) {
    if (media.capacity() < PhysicalSectorSize) return {};
    std::array<std::uint8_t, PhysicalSectorSize> root;
    media.readExactly(0, root, token);
    const auto layout = probe(root, media.capacity());
    if (!layout) return {};
    const bool ide = *layout == Layout::IdeWordSwapped;
    if (ide) swapWords(root);
    const auto sectors = diskSectors(root, media.capacity());
    VolumeDetectionResult result;
    const auto append = [&](const Partition& entry, std::uint64_t start) {
        const bool fat = entry.type == GemPartition || entry.type == BigGemPartition;
        result.volumes.emplace_back(start * PhysicalSectorSize, entry.count * PhysicalSectorSize,
            std::string(VolumeConstants::Partition), std::string(ide ? IdeTableId : AhdiTableId),
            std::int32_t(result.volumes.size() + 1), std::nullopt, std::nullopt,
            fat ? std::optional<std::string>(Fat16Id) : std::nullopt, entry.type);
    };
    for (std::size_t i = 0; i < PartitionCount; ++i) {
        checkCancellation(token);
        const auto entry = partition(root, i);
        if (!entry || !fits(*entry, sectors)) continue;
        if (entry->type != ExtendedPartition) {
            append(*entry, entry->start);
            continue;
        }
        const auto base = entry->start;
        const auto end = base + entry->count;
        std::set<std::uint64_t> visited;
        auto current = base;
        while (current < end && visited.insert(current).second) {
            checkCancellation(token);
            std::array<std::uint8_t, PhysicalSectorSize> auxiliary;
            media.readExactly(current * PhysicalSectorSize, auxiliary, token);
            if (ide) swapWords(auxiliary);
            const auto logical = partition(auxiliary, 0);
            if (!logical || logical->type == ExtendedPartition || !fits(*logical, end - current)) break;
            append(*logical, current + logical->start);
            const auto next = partition(auxiliary, 1);
            if (!next || next->type != ExtendedPartition || !fits(*next, entry->count)) break;
            current = base + next->start;
        }
    }
    return result;
}

std::optional<std::vector<std::uint8_t>> prepare(std::uint64_t capacity, Layout layout, std::stop_token token) {
    checkCancellation(token);
    constexpr auto VolumeOffset = FirstPartitionSector * PhysicalSectorSize;
    if (capacity < VolumeOffset + MinimumVolume || capacity > VolumeOffset + MaximumVolume || capacity % PhysicalSectorSize)
        return {};
    const auto volume = capacity - VolumeOffset;

    auto sectorBytes = PhysicalSectorSize;
    while (volume / sectorBytes > MaxBpbSectors) sectorBytes *= 2;
    const auto sectors = std::uint32_t(volume / sectorBytes);
    const auto rootSectors = (RootEntries * EntryBytes + sectorBytes - 1) / sectorBytes;
    std::uint32_t fatSectors = 1;
    while (((sectors - ReservedSectors - rootSectors - FatCount * fatSectors) / ClusterSectors + FatCount) *
        FatEntryBytes > fatSectors * sectorBytes) ++fatSectors;

    std::vector<std::uint8_t> data(VolumeOffset + (ReservedSectors + FatCount * fatSectors + rootSectors) * sectorBytes);
    auto root = MutableBytes(data).first(PhysicalSectorSize);
    Binary::writeBig(root, DiskSize, std::uint32_t(capacity / PhysicalSectorSize));
    root[PartitionFlags] = PartitionPresent;
    const auto type = volume < BigPartition ? GemPartition : BigGemPartition;
    std::copy(type.begin(), type.end(), root.begin() + PartitionType);
    Binary::writeBig(root, PartitionStart, FirstPartitionSector);
    Binary::writeBig(root, PartitionSize, std::uint32_t(volume / PhysicalSectorSize));

    auto boot = MutableBytes(data).subspan(VolumeOffset, sectorBytes);
    std::copy(BootBranch.begin(), BootBranch.end(), boot.begin());
    std::copy(BootName.begin(), BootName.end(), boot.begin() + BootNameOffset);
    setBpb(boot, BpbSectorBytes, sectorBytes);
    boot[BpbCluster] = ClusterSectors;
    setBpb(boot, BpbReserved, ReservedSectors);
    boot[BpbFatCount] = FatCount;
    setBpb(boot, BpbRootEntries, RootEntries);
    setBpb(boot, BpbTotal, sectors);
    boot[BpbMedia] = FixedMedia;
    setBpb(boot, BpbFatSectors, fatSectors);
    setBpb(boot, BpbTrackSectors, SectorsPerTrack);
    setBpb(boot, BpbHeads, Heads);
    for (std::uint32_t i = 0; i < FatCount; ++i) {
        auto fat = MutableBytes(data).subspan(VolumeOffset + (ReservedSectors + i * fatSectors) * sectorBytes, fatSectors * sectorBytes);
        Binary::writeLittle(fat, 0, MediaEntry);
        Binary::writeLittle(fat, FatEntryBytes, EndOfChain);
    }

    if (layout == Layout::IdeWordSwapped) swapWords(data);
    checkCancellation(token);
    return data;
}
}
