#include <RetroMediaLab/MediaFileSystems/Rdb.h>
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include "Helpers.h"
#include <QString>
#include <limits>
#include <set>

namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace Rdb;
constexpr std::uint32_t EndOfChain = std::numeric_limits<std::uint32_t>::max();
constexpr std::uint32_t GeometryLimit = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint32_t HostId = 7, RigidDiskFlags = 7, ChecksumWords = 64;
constexpr std::uint32_t BootableFlag = 1, EnvironmentWords = 16, SectorWords = SectorSize / sizeof(std::uint32_t);
constexpr std::uint32_t SectorsPerBlock = 1, ReservedPartitionBlocks = 2, BufferCount = 30;
constexpr std::uint32_t MaxTransfer = 0x1fe00, DmaMask = 0x7ffffffe;
constexpr auto RigidDiskSignature = "RDSK";
constexpr auto PartitionSignature = "PART";
constexpr std::string_view DiskVendor = "RetroMed", DiskProduct = "Virtual HDD     ";
constexpr int NameLimit = 30, MinNameCharacter = 32, MaxNameCharacter = 255;
constexpr int WordCountOffset = 4, ChecksumOffset = 8, HostOffset = 12;
constexpr int BlockBytesOffset = 16, FlagsOffset = 20, BadBlockListOffset = 24, PartListOffset = 28;
constexpr int FileSystemListOffset = 32, DriveInitOffset = 36;
constexpr int CylindersOffset = 64, SectorsOffset = 68, HeadsOffset = 72;
constexpr int ReservedEndOffset = 132, LowCylinderOffset = 136, HighCylinderOffset = 140;
constexpr int CylinderBlocksOffset = 144, HighestRdbBlockOffset = 152, VendorOffset = 160, ProductOffset = 168;
constexpr int NextPartOffset = 16, PartNameLengthOffset = 36, PartNameOffset = 37;
constexpr int EnvSizeOffset = 128, EnvSectorWordsOffset = 132, EnvHeadsOffset = 140;
constexpr int EnvSectorsPerBlockOffset = 144, EnvSectorsOffset = 148, EnvReservedOffset = 152;
constexpr int EnvLowCylinderOffset = 164, EnvHighCylinderOffset = 168, EnvBuffersOffset = 172;
constexpr int EnvMaxTransferOffset = 180, EnvDmaMaskOffset = 184, EnvDosTypeOffset = 192;
constexpr std::size_t WordBytes = sizeof(std::uint32_t);
using Record = std::array<std::uint8_t, SectorSize>;

std::uint32_t word(Bytes bytes, std::size_t offset) {
    return Binary::big<std::uint32_t>(bytes, offset);
}

bool validRecord(Bytes bytes, std::size_t requiredBytes) {
    const auto count = word(bytes, WordCountOffset);
    if (count < requiredBytes / WordBytes || count > bytes.size() / WordBytes) return false;
    std::uint32_t sum = 0;
    for (std::uint32_t i = 0; i < count; ++i) sum += word(bytes, i * WordBytes);
    return sum == 0;
}

void put(MutableBytes bytes, std::size_t offset, std::uint32_t value) {
    Binary::writeBig(bytes, offset, value);
}

void initializeBlock(MutableBytes bytes, std::string_view signature) {
    std::copy(signature.begin(), signature.end(), bytes.begin());
    put(bytes, WordCountOffset, ChecksumWords);
    put(bytes, HostOffset, HostId);
}

void checksum(MutableBytes bytes) {
    std::uint32_t sum = 0;
    for (std::uint32_t i = 0; i < ChecksumWords; ++i)
        sum += Binary::big<std::uint32_t>(bytes, i * sizeof(std::uint32_t));
    put(bytes, ChecksumOffset, std::uint32_t(0) - sum);
}
}

std::optional<std::vector<std::uint8_t>> Rdb::prepare(std::uint64_t capacity, std::span<const Partition> partitions,
        std::uint32_t heads, std::uint32_t sectorsPerTrack, std::stop_token token) {
    checkCancellation(token);
    if (!heads || heads > GeometryLimit || !sectorsPerTrack || sectorsPerTrack > GeometryLimit ||
        capacity % SectorSize || capacity / SectorSize > EndOfChain || partitions.size() >= capacity / SectorSize)
        return {};
    const auto cylinderBlocks = heads * sectorsPerTrack;
    const auto cylinderBytes = std::uint64_t(cylinderBlocks) * SectorSize;
    const auto blockCount = std::uint64_t(partitions.size()) + 1;
    const auto reserved = (blockCount + cylinderBlocks - 1) / cylinderBlocks;
    const auto cylinders = capacity / cylinderBytes;
    if (capacity % cylinderBytes || cylinders <= reserved ||
        blockCount > std::numeric_limits<std::size_t>::max() / SectorSize)
        return {};

    std::vector<const Partition*> sorted;
    sorted.reserve(partitions.size());
    for (const auto& partition : partitions) sorted.push_back(&partition);
    std::ranges::sort(sorted, {}, [](const Partition* partition) { return partition->offset; });
    auto end = reserved * cylinderBytes;
    for (const auto* partition : sorted) {
        checkCancellation(token);
        if (partition->offset < end || partition->offset % cylinderBytes || !partition->length ||
            partition->length % cylinderBytes || partition->offset > capacity || partition->length > capacity - partition->offset)
            return {};
        end = partition->offset + partition->length;
    }

    std::vector<std::uint8_t> table(std::size_t(blockCount * SectorSize));
    const auto block = [&](std::size_t index) { return MutableBytes(table).subspan(index * SectorSize, SectorSize); };
    auto rdb = block(0);
    initializeBlock(rdb, RigidDiskSignature);
    put(rdb, BlockBytesOffset, SectorSize);
    put(rdb, FlagsOffset, RigidDiskFlags);
    put(rdb, BadBlockListOffset, EndOfChain);
    put(rdb, PartListOffset, partitions.empty() ? EndOfChain : 1);
    put(rdb, FileSystemListOffset, EndOfChain);
    put(rdb, DriveInitOffset, EndOfChain);
    put(rdb, CylindersOffset, std::uint32_t(cylinders));
    put(rdb, SectorsOffset, sectorsPerTrack);
    put(rdb, HeadsOffset, heads);
    put(rdb, ReservedEndOffset, std::uint32_t(reserved * cylinderBlocks - 1));
    put(rdb, LowCylinderOffset, std::uint32_t(reserved));
    put(rdb, HighCylinderOffset, std::uint32_t(cylinders - 1));
    put(rdb, CylinderBlocksOffset, cylinderBlocks);
    put(rdb, HighestRdbBlockOffset, std::uint32_t(partitions.size()));
    std::copy(DiskVendor.begin(), DiskVendor.end(), rdb.begin() + VendorOffset);
    std::copy(DiskProduct.begin(), DiskProduct.end(), rdb.begin() + ProductOffset);
    checksum(rdb);

    for (std::size_t i = 0; i < partitions.size(); ++i) {
        checkCancellation(token);
        const auto& partition = partitions[i];
        if (partition.name.size() > NameLimit * 2) return {};
        const auto name = QString::fromUtf8(partition.name);
        if (name.isEmpty() || name.size() > NameLimit ||
            std::ranges::any_of(name, [](QChar c) { return c.unicode() < MinNameCharacter || c.unicode() > MaxNameCharacter; }))
            return {};
        const auto latin1 = name.toLatin1();
        auto part = block(i + 1);
        initializeBlock(part, PartitionSignature);
        put(part, NextPartOffset, i + 1 < partitions.size() ? std::uint32_t(i + 2) : EndOfChain);
        put(part, FlagsOffset, partition.active ? BootableFlag : 0);
        part[PartNameLengthOffset] = std::uint8_t(latin1.size());
        std::copy(latin1.begin(), latin1.end(), part.begin() + PartNameOffset);
        put(part, EnvSizeOffset, EnvironmentWords);
        put(part, EnvSectorWordsOffset, SectorWords);
        put(part, EnvHeadsOffset, heads);
        put(part, EnvSectorsPerBlockOffset, SectorsPerBlock);
        put(part, EnvSectorsOffset, sectorsPerTrack);
        put(part, EnvReservedOffset, ReservedPartitionBlocks);
        put(part, EnvLowCylinderOffset, std::uint32_t(partition.offset / cylinderBytes));
        put(part, EnvHighCylinderOffset, std::uint32_t((partition.offset + partition.length) / cylinderBytes - 1));
        put(part, EnvBuffersOffset, BufferCount);
        put(part, EnvMaxTransferOffset, MaxTransfer);
        put(part, EnvDmaMaskOffset, DmaMask);
        put(part, EnvDosTypeOffset, partition.dosType);
        checksum(part);
    }
    return table;
}

bool Rdb::validHeader(Bytes bytes) {
    const std::string_view signature = RigidDiskSignature;
    if (bytes.size() < SectorSize || !std::equal(signature.begin(), signature.end(), bytes.begin()) ||
        !validRecord(bytes.first(SectorSize), PartListOffset + WordBytes)) return false;
    const auto blockBytes = word(bytes, BlockBytesOffset);
    return blockBytes >= SectorSize && blockBytes % SectorSize == 0 && blockBytes <= VolumeConstants::MaximumBlockSize;
}

std::optional<VolumeDetectionResult> detectRdb(const BlockMedia& media, std::stop_token token) {
    checkCancellation(token);
    if (!media.isHardDisk()) return {};
    Record header{};
    std::uint64_t headerOffset = 0;
    bool found = false;
    bool recognized = false;
    const std::string_view signature = RigidDiskSignature;
    for (std::uint64_t i = 0; i < std::min<std::uint64_t>(HeaderSearchSectors, media.capacity() / SectorSize); ++i) {
        headerOffset = i * SectorSize;
        media.readExactly(headerOffset, header, token);
        if (!std::equal(signature.begin(), signature.end(), header.begin())) continue;
        recognized = true;
        if (validHeader(header)) {
            found = true;
            break;
        }
    }
    if (!recognized) return {};
    VolumeDetectionResult result;
    const auto invalid = [&](DiagnosticCode code) { result.diagnostics.push_back({code, {}}); };
    const auto blockBytes = word(header, BlockBytesOffset);
    if (!found) {
        invalid(DiagnosticCode::InvalidSource);
        return result;
    }

    std::set<std::uint64_t> visited{headerOffset};
    Record part{};
    const std::string_view partSignature = PartitionSignature;
    auto next = word(header, PartListOffset);
    for (std::int32_t number = 1; next && next != EndOfChain; ++number) {
        checkCancellation(token);
        const auto address = std::uint64_t(next) * blockBytes;
        if (address > media.capacity() - SectorSize || number == std::numeric_limits<std::int32_t>::max()) {
            invalid(DiagnosticCode::ReadOutOfBounds);
            break;
        }
        if (!visited.insert(address).second) {
            invalid(DiagnosticCode::InvalidSource);
            break;
        }
        media.readExactly(address, part, token);
        if (!std::equal(partSignature.begin(), partSignature.end(), part.begin()) ||
            !validRecord(part, EnvHighCylinderOffset + WordBytes)) {
            invalid(DiagnosticCode::InvalidSource);
            break;
        }
        next = word(part, NextPartOffset);

        constexpr std::uint32_t RequiredEnv = (EnvHighCylinderOffset - EnvSizeOffset) / WordBytes;
        const auto env = word(part, EnvSizeOffset);
        const auto first = word(part, EnvLowCylinderOffset);
        const auto last = word(part, EnvHighCylinderOffset);
        const auto cylinderBlocks = std::uint64_t(word(part, EnvHeadsOffset)) * word(part, EnvSectorsOffset);
        if (env < RequiredEnv || first > last || !cylinderBlocks || cylinderBlocks > media.capacity() / blockBytes) {
            invalid(DiagnosticCode::InvalidSource);
            continue;
        }
        const auto cylinderBytes = cylinderBlocks * blockBytes;
        const auto end = std::uint64_t(last) + 1;
        if (end > media.capacity() / cylinderBytes || end > std::uint64_t(std::numeric_limits<std::int64_t>::max()) / cylinderBytes) {
            invalid(DiagnosticCode::ReadOutOfBounds);
            continue;
        }
        const auto length = std::min<std::uint8_t>(part[PartNameLengthOffset], NameLimit);
        auto name = QString::fromLatin1(reinterpret_cast<const char*>(part.data() + PartNameOffset), length).toStdString();
        std::optional<std::string> fileSystem, type;
        if (env >= EnvironmentWords && word(part, WordCountOffset) >= (EnvDosTypeOffset + WordBytes) / WordBytes) {
            const auto dosType = word(part, EnvDosTypeOffset);
            type = (QStringLiteral("0x") + QString::number(dosType, 16).rightJustified(8, u'0')).toStdString();
            if (dosType >= DosOfs && dosType <= DosOfs + std::uint8_t(AmigaDos::Variant::FfsLongNames))
                fileSystem = AmigaDos::fileSystemId(AmigaDos::Variant(dosType - DosOfs));
        }
        result.volumes.emplace_back(std::int64_t(first * cylinderBytes), std::int64_t((end - first) * cylinderBytes),
            std::string(VolumeConstants::Partition), std::string(VolumeConstants::Rdb), number,
            std::nullopt, std::nullopt, std::move(fileSystem), std::move(type), std::nullopt, std::move(name));
    }
    Binary::overlaps(result, DiagnosticCode::InvalidSource);
    return result;
}
}
