#include "BlockVolume.h"
#include "../../FileSystems/Fat/Fat12/Fat12Internal.h"
#include "../../Primitives/Binary.h"
#include <bit>

namespace RetroMediaLab::MediaFileSystems::Composition {
namespace {
using namespace AtariHardDisk;
using namespace FatBootSectorLayout;
using Fat12::Internal::TableKind;
constexpr auto Table = TableKind::Fat16;
constexpr std::uint32_t MaximumSectorSize = 32768, MaximumClusterSectors = 128, MaximumFatCopies = 2;

struct FatVolume {
    BlockVolume image;
    std::shared_ptr<const SectorBlock> boot;
    Fat12::Layout layout;
    std::uint8_t descriptor;
};

std::optional<FatVolume> parse(const ImageDocument& document, const VolumeDescriptor& volume, std::stop_token token) {
    if (!document.isHardDisk() || volume.fileSystemId() != Fat16Id) return {};
    const bool ide = volume.partitionTable() == IdeTableId;
    if (!ide && volume.partitionTable() != AhdiTableId) return {};
    const auto* blocks = dynamic_cast<const BlockRepresentation*>(&document.representation());
    if (!blocks || blocks->capacity() < 0) return {};
    const auto capacity = std::uint64_t(blocks->capacity());
    if (volume.start() > capacity || volume.length() > capacity - volume.start() ||
        volume.length() < PhysicalSectorSize || volume.start() % PhysicalSectorSize) return {};

    const auto boot = BlockVolume(*blocks, volume, Fat16Id, PhysicalSectorSize, ide).block(0, token);
    const Bytes bpb(boot->data);
    const auto word = [bpb](auto offset) { return std::uint32_t(Binary::little<std::uint16_t>(bpb, offset)); };
    const auto sectorSize = word(BytesPerSectorOffset);
    const auto clusterSize = bpb[SectorsPerClusterOffset];
    const auto reserved = word(ReservedSectorCountOffset);
    const auto copies = bpb[FatCountOffset];
    const auto rootEntries = word(RootEntryCountOffset);
    const auto fatSize = word(SectorsPerFatOffset);
    const auto descriptor = bpb[MediaDescriptorOffset];
    auto sectors = word(TotalSectors16Offset);
    if (!sectors) sectors = Binary::little<std::uint32_t>(bpb, TotalSectors32Offset);

    if (sectorSize < PhysicalSectorSize || sectorSize > MaximumSectorSize || !std::has_single_bit(sectorSize) ||
        clusterSize > MaximumClusterSectors || !std::has_single_bit(clusterSize) || sectors > volume.length() / sectorSize ||
        !reserved || !rootEntries || !fatSize || !copies || copies > MaximumFatCopies ||
        descriptor < Fat12Constants::Fat12Table::MinimumMediaDescriptor) return {};
    constexpr auto EntryBytes = FatConstants::FatDirectoryLayout::EntrySize;
    const auto rootStart = reserved + copies * fatSize;
    const auto rootSize = (rootEntries * EntryBytes + sectorSize - 1) / sectorSize;
    const auto dataStart = rootStart + rootSize;
    if (dataStart >= sectors) return {};
    const auto clusters = (sectors - dataStart) / clusterSize;
    constexpr auto FirstData = Fat12Constants::Fat12Table::FirstDataCluster;
    if (!clusters || clusters > Fat12::Internal::Fat16FirstReserved - FirstData ||
        std::uint64_t(fatSize) * sectorSize < std::uint64_t(clusters + FirstData) * sizeof(std::uint16_t)) return {};
    return FatVolume{BlockVolume(*blocks, volume, Fat16Id, sectorSize, ide), boot,
        {reserved, fatSize, rootStart, rootSize, dataStart, clusterSize, clusters, copies}, descriptor};
}

class AtariFatDocumentReader final : public MediaFileSystemReader {
public:
    std::string_view id() const noexcept override { return Fat16Id; }
    bool canRead(const ImageDocument& document, const VolumeDescriptor& volume, std::stop_token token) const override {
        const auto parsed = parse(document, volume, token);
        if (!parsed) return false;
        for (std::uint32_t copy = 0; copy < parsed->layout.fatCount; ++copy) {
            const auto fat = parsed->image.block(parsed->layout.reservedSectors + copy * parsed->layout.sectorsPerFat, token);
            if (fat && Fat12::Internal::usableTable(fat->data, parsed->descriptor, parsed->layout.clusterCount, Table))
                return true;
        }
        return false;
    }
    FileSystemVolume read(const ImageDocument& document, const VolumeDescriptor& volume, std::stop_token token) const override {
        const auto parsed = parse(document, volume, token);
        if (!parsed) throw MediaError(DiagnosticCode::FatUnsupportedBoot, {std::string(Fat16Id)});
        return Fat12::Internal::readVolume(parsed->image, parsed->boot, parsed->layout, parsed->descriptor, Table, token);
    }
};
}

std::shared_ptr<const MediaFileSystemReader> createAtariFatDocumentReader() {
    return std::make_shared<AtariFatDocumentReader>();
}
}
