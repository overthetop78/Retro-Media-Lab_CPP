#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Fat12Internal.h"
#include <RetroMediaLab/MediaFileSystems/AtariHardDisk.h>
namespace RetroMediaLab::MediaFileSystems::Fat12 {
using namespace FatConstants;
using namespace Fat12Constants;
namespace {
std::optional<std::uint8_t> descriptor(const SectorImage& image,const SectorBlock& boot,const Layout& layout,std::stop_token token) {
    if(Internal::dosPlus(image))return boot.data.front();
    if(Internal::same(image.geometry().formatId,Fat12Formats::ApricotPcXi315)) {
        const auto firstFat=image.block(layout.reservedSectors,token);if(!firstFat || firstFat->data.empty())return std::nullopt;return firstFat->data[FatBootSectorLayout::FatMediaDescriptorDataOffset];
    }
    return boot.data[FatBootSectorLayout::MediaDescriptorOffset];
}
}
bool canRead(const SectorImage& image,std::stop_token token) {
    checkCancellation(token);
    const auto size=Internal::dosPlus(image)?Values::DosPlusSectorSize:FatBootSectorLayout::SectorSize;
    if(!Internal::supported(image.geometry().formatId) || image.geometry().blockSize!=size)return false;
    const auto boot=image.block(0,token);if(!boot || boot->data.size()<FatBootSectorLayout::ExtendedBootMinimumLength)return false;
    const auto layout=readLayout(boot->data,image.blockCount(),image.geometry().formatId);if(!layout)return false;
    const auto media=descriptor(image,*boot,*layout,token);if(!media)return false;
    for(std::uint32_t copy=0;copy<layout->fatCount;++copy) {
        std::vector<Diagnostic> ignored;const auto range=Internal::sectors(image,layout->reservedSectors+copy*layout->sectorsPerFat,layout->sectorsPerFat,ignored,token);
        if(range.complete() && Internal::usableTable(range.bytes,*media,layout->clusterCount))return true;
    }
    return false;
}
FileSystemVolume read(const SectorImage& image,std::stop_token token) {
    checkCancellation(token);
    const auto boot=image.block(0,token);if(!boot)throw Internal::unsupported(image,boot);
    const auto layout=readLayout(boot->data,image.blockCount(),image.geometry().formatId);if(!layout)throw Internal::unsupported(image,boot);
    if(boot->data.size()<=FatBootSectorLayout::MediaDescriptorOffset && !Internal::dosPlus(image))throw Internal::unsupported(image,boot);
    const auto media=descriptor(image,*boot,*layout,token);if(!media)throw Internal::unsupported(image,boot);
    return Internal::readVolume(image,boot,*layout,*media,Internal::TableKind::Fat12,token);
}

FileSystemVolume Internal::readVolume(const SectorImage& image,const std::shared_ptr<const SectorBlock>& boot,const Layout& layout,
    std::uint8_t media,TableKind kind,std::stop_token token) {
    FileSystemVolume volume;
    if (kind == TableKind::Fat16) {
        constexpr std::string_view Fat16DisplayName = "FAT16";
        volume.fileSystemId = AtariHardDisk::Fat16Id;
        volume.fileSystemDisplayName = Fat16DisplayName;
    } else {
        volume.fileSystemId = Values::FileSystemId;
        applyFileSystemDisplayName(volume,image.geometry());
    }
    volume.capacity=image.capacity();
    const auto fat=bestTable(image,layout,media,volume.warnings,token,kind);
    if(!fat.complete() || !usableTable(fat.bytes,media,layout.clusterCount,kind)) {
        throw unsupported(image,boot);
    }
    const auto root=sectors(image,layout.rootStart,layout.rootSectors,volume.warnings,token);
    const auto budget=std::min(Values::MaximumDecodedBytes,std::uint64_t(layout.clusterCount)*layout.sectorsPerCluster*image.geometry().blockSize);
    Internal::ReadContext context{image,volume.warnings,token,budget,{},kind};
    volume.entries=Internal::directory(context,root,fat,layout,0,{});
    std::uint64_t freeClusters=0;
    if(fat.complete())for(auto cluster=Fat12Table::FirstDataCluster;cluster<Fat12Table::FirstDataCluster+layout.clusterCount;++cluster) {
        checkCancellation(token);const auto value=clusterEntry(fat.bytes,cluster,kind);if(value && *value==Fat12Table::FreeCluster)++freeClusters;
    }
    volume.freeBytes=freeClusters*layout.sectorsPerCluster*image.geometry().blockSize;volume.freeSpaceKnown=fat.valid();
    const auto label=Fat::volumeLabel(root.bytes);volume.name=label?*label:Internal::dosPlus(image)?std::string{}:Fat::bootVolumeLabel(boot->data);
    return volume;
}
}
