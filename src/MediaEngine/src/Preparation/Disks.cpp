#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include "../Composition/Runtime.h"
#include "../Models/Sectors/Representation.h"
#include "../Images/Creation/Blank.h"
#include "../Formats/HardDisk/MfmEmu/Adapter.h"
#include "../Formats/HardDisk/Raw/Constants.h"
#include "../Formats/HardDisk/Raw/Writer.h"
#include "../Formats/HardDisk/MfmEmu/Constants.h"
#include "../Formats/HardDisk/Acorn/Container.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include "../Formats/HardDisk/Chd/Constants.h"
#include "../Formats/HardDisk/ContainerConstants.h"
#include "../Formats/HardDisk/Qcow2/Constants.h"
#include "../Formats/HardDisk/Vhd/Constants.h"
#include "../Formats/HardDisk/Vhdx/Constants.h"
#include "../Formats/HardDisk/Vdi/Constants.h"
#include "../Formats/HardDisk/Vmdk/Constants.h"
#include "../Sources/AtomicImageSetWriter.h"
#include "../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include <RetroMediaLab/MediaFileSystems/AtariHardDisk.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <RetroMediaLab/MediaFileSystems/Rdb.h>
#include <limits>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;namespace A=AcornWinchester::Constants;
namespace {
std::shared_ptr<const MediaSource> source(const std::filesystem::path& path){SourceOptions options;const auto utf8=path.u8string();options.primaryPath.assign(utf8.begin(),utf8.end());return std::make_shared<MediaSource>(std::move(options));}

std::vector<DataRange> containerRanges(const HardDiskGeometry& geometry,
        const std::optional<std::string>& fileSystem, std::stop_token token) {
    if (!fileSystem) return {DataRange(0, geometry.capacity(), DataRangeKind::Zero)};
    const bool atariIde = *fileSystem == HardDiskPreparationIds::AtariIdeFat16;
    if (atariIde || *fileSystem == HardDiskPreparationIds::AtariAhdiFat16) {
        using namespace M::AtariHardDisk;
        if (geometry.sectorSize() != PhysicalSectorSize) throw MediaError(DiagnosticCode::DiskPreparationRequest);
        const auto prefix = prepare(geometry.capacity(), atariIde ? Layout::IdeWordSwapped : Layout::Ahdi, token);
        if (!prefix) throw MediaError(DiagnosticCode::DiskPreparationRequest);
        const auto length = std::int64_t(prefix->size());
        return {
            DataRange(0, length, DataRangeKind::Stored, memorySource(*prefix)),
            DataRange(length, geometry.capacity() - length, DataRangeKind::Zero)
        };
    }
    constexpr auto VolumeLabel = "RetroMediaLab";
    constexpr int AmigaSectorSize = 512;
    using M::AmigaDos::Variant;
    constexpr std::array variants{Variant::Ofs, Variant::Ffs, Variant::OfsInternational,
        Variant::FfsInternational, Variant::OfsDirectoryCache, Variant::FfsDirectoryCache};
    const bool rdbOfs = *fileSystem == HardDiskPreparationIds::AmigaRdbOfs;
    const bool rdbFfs = *fileSystem == HardDiskPreparationIds::AmigaRdbFfs;
    const auto variant = std::ranges::find_if(variants, [&](Variant candidate) {
        if (rdbOfs || rdbFfs) return candidate == (rdbFfs ? Variant::Ffs : Variant::Ofs);
        return *fileSystem == M::AmigaDos::fileSystemId(candidate);
    });
    if (variant == variants.end() || geometry.sectorSize() != AmigaSectorSize)
        throw MediaError(DiagnosticCode::DiskPreparationRequest);
    constexpr auto FirstPartitionOffset = M::Rdb::DefaultHeads * M::Rdb::DefaultSectorsPerTrack * M::Rdb::SectorSize;
    const std::int64_t start = rdbOfs || rdbFfs ? FirstPartitionOffset : 0;
    const auto volumeCapacity = geometry.capacity() - start;
    std::vector<DataRange> ranges;
    if (start) {
        if (volumeCapacity <= 0) throw MediaError(DiagnosticCode::DiskPreparationRequest);
        constexpr auto PartitionName = "DH0";
        const auto partition = M::Rdb::Partition{std::uint64_t(start), std::uint64_t(volumeCapacity),
            PartitionName, rdbFfs ? M::Rdb::DosFfs : M::Rdb::DosOfs, true};
        const auto table = M::Rdb::prepare(geometry.capacity(), std::span(&partition, 1),
            M::Rdb::DefaultHeads, M::Rdb::DefaultSectorsPerTrack, token);
        if (!table) throw MediaError(DiagnosticCode::DiskPreparationRequest);
        const auto tableSize = std::int64_t(table->size());
        ranges.emplace_back(0, tableSize, DataRangeKind::Stored, memorySource(*table));
        ranges.emplace_back(tableSize, start - tableSize, DataRangeKind::Zero);
    }
    const auto volume = M::AmigaDos::prepareHardDiskVolume(volumeCapacity, *variant, VolumeLabel, token);
    const auto bootSize = std::int64_t(volume.boot.size());
    const auto metadataOffset = start + std::int64_t(volume.metadataOffset);
    const auto metadataSize = std::int64_t(volume.metadata.size());
    const auto metadataEnd = metadataOffset + metadataSize;
    ranges.emplace_back(start, bootSize, DataRangeKind::Stored, memorySource(volume.boot));
    ranges.emplace_back(start + bootSize, metadataOffset - start - bootSize, DataRangeKind::Zero);
    ranges.emplace_back(metadataOffset, metadataSize, DataRangeKind::Stored, memorySource(volume.metadata));
    ranges.emplace_back(metadataEnd, geometry.capacity() - metadataEnd, DataRangeKind::Zero);
    return ranges;
}

void createContainer(const std::filesystem::path& path, const std::string& format,
        const HardDiskGeometry& geometry, const std::optional<std::string>& fileSystem, bool preallocate, std::stop_token token) {
    using namespace HardDiskContainers::Constants;
    constexpr auto StagingName = ".rml-disk-XXXXXX";
    const bool supportsFixedVariant = format == Vhd::Constants::FormatId || format == Vhdx::Constants::FormatId ||
        format == Vdi::Constants::FormatId || format == Vmdk::Constants::FormatId;
    const bool raw = format == HardDiskRaw::Constants::FormatId;
    if (!raw && !supportsFixedVariant && (preallocate || (format != Qcow2::Constants::FormatId && format != ChdHardDisk::Constants::FormatId)))
        throw MediaError(DiagnosticCode::DiskPreparationRequest);
    const QFileInfo target(QString::fromStdU16String(path.u16string()));
    if (target.fileName().isEmpty()) throw MediaError(DiagnosticCode::DiskPreparationRequest);
    if (target.exists() || target.isSymLink())
        throw MediaError(DiagnosticCode::AtomicSetExists, {target.absoluteFilePath().toStdString()});
    if (!QDir().mkpath(target.absolutePath())) throw MediaError(DiagnosticCode::AtomicOpen, {target.absolutePath().toStdString()});
    QTemporaryDir staging(target.dir().filePath(QLatin1String(StagingName)));
    if (!staging.isValid()) throw MediaError(DiagnosticCode::AtomicOpen, {target.absolutePath().toStdString()});
    const auto preparedPath = std::filesystem::path(staging.filePath(target.fileName()).toStdU16String());
    auto blocks = std::make_shared<BlockImage>(geometry.capacity(), geometry.sectorSize(),
        containerRanges(geometry, fileSystem, token), geometry);
    std::map<std::string, std::string> metadata;
    if (preallocate && supportsFixedVariant) metadata.emplace(VariantKey, FixedVariant);
    auto document = std::make_shared<MediaDocument>(source(path), format, MediaKind::HardDisk, blocks,
        std::vector<M::VolumeDescriptor>{}, std::vector<M::MessageRecipe>{}, std::move(metadata));
    if (raw && !preallocate) {
        const auto extension = path.extension().u8string();
        if (!HardDiskRaw::Writer{}.canWrite(*document, format, std::string(extension.begin(), extension.end())))
            throw MediaError(DiagnosticCode::HardDiskRawWrite);
        writeAtomicSeekableImage(preparedPath, [&](const auto& output, std::stop_token stop) {
            for (const auto& range : blocks->ranges()) {
                if (range.kind() != DataRangeKind::Stored) continue;
                output.seek(std::uint64_t(range.address()));
                copyStoredRange(range, output.write, stop);
            }
            // Extend the image through the final unwritten sector.
            output.seek(std::uint64_t(geometry.capacity() - 1));
            output.write(std::array<std::uint8_t, 1>{});
        }, token);
        publishNewImageSet(std::array{preparedPath}, preparedPath.parent_path(), target.absolutePath().toStdU16String(), token);
        return;
    }
    auto files = RuntimeDetails::defaultServices()->writing.writingService().writeAsync(
        std::move(document), preparedPath, format, token).get();
    // Expose accompanying extents before the descriptor that references them.
    std::stable_partition(files.begin(), files.end(), [&](const auto& file) { return file != preparedPath; });
    publishNewImageSet(files, preparedPath.parent_path(), target.absolutePath().toStdU16String(), token);
}
}
std::future<void> createBlankDiskAsync(std::filesystem::path path,std::string format,std::stop_token token){
    return std::async(std::launch::async,[path=std::move(path),format=std::move(format),token]{
        checkCancellation(token);auto image=BlankSectorImageFactory::create(format,token);auto document=std::make_shared<MediaDocument>(source(path),format,MediaKind::Floppy,std::make_shared<SectorRepresentation>(std::move(image)));
        RuntimeDetails::defaultServices()->writing.writingService().writeAsync(std::move(document),path,format,token).get();
    });
}
std::future<void> createHardDiskAsync(std::filesystem::path path,std::string format,HardDiskGeometry geometry,std::optional<std::string> fileSystemId,std::stop_token token,bool preallocate){
    return std::async(std::launch::async,[path=std::move(path),format=std::move(format),geometry,fileSystemId=std::move(fileSystemId),token,preallocate]{
        checkCancellation(token);if(format==MfmEmu::Constants::FormatId){MfmEmu::createAsync(path,geometry,fileSystemId,token,preallocate).get();return;}
        if (format != A::FormatId) {
            createContainer(path, format, geometry, fileSystemId, preallocate, token);
            return;
        }
        if(!AcornWinchester::supportsGeometry(geometry))throw MediaError(DiagnosticCode::AcornDscGeometry);
        std::vector<DataRange> ranges;std::int64_t position=0;
        if(fileSystemId){
            if(geometry.sectorSize()!=A::SectorSize||*fileSystemId!=M::FileSystemIds::AcornAdfs||geometry.logicalSectorCount()>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::DiskPreparationRequest);
            const auto header=M::Adfs::createBbcHeader(static_cast<std::uint32_t>(geometry.logicalSectorCount()));position=static_cast<std::int64_t>(header.size());ranges.emplace_back(0,position,DataRangeKind::Stored,memorySource(header));
        }
        if(position<geometry.capacity())ranges.emplace_back(position,geometry.capacity()-position,DataRangeKind::Zero);
        auto blocks=std::make_shared<BlockImage>(geometry.capacity(),geometry.sectorSize(),std::move(ranges),geometry);auto document=std::make_shared<MediaDocument>(source(path),format,MediaKind::HardDisk,std::move(blocks));
        AcornWinchester::Writer{}.createAsync(std::move(document),path,format,token,preallocate).get();
    });
}
}
