#include "CatalogConstants.h"
#include "BlockVolume.h"
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include <RetroMediaLab/MediaFileSystems/SectorReader.h>
#include "../../FileSystems/Amiga/Dos/Internal.h"
#include <algorithm>

namespace RetroMediaLab::MediaFileSystems::Composition {
namespace {
using namespace AmigaDos::Constants;

std::optional<BlockVolume> blockVolume(const ImageDocument& document, const VolumeDescriptor& volume) {
    const auto* blocks = dynamic_cast<const BlockRepresentation*>(&document.representation());
    if (!document.isHardDisk() || !blocks || blocks->capacity() < 0) return {};
    const auto capacity = std::uint64_t(blocks->capacity());
    if (volume.start() % BlockSize || volume.length() % BlockSize || volume.length() < BootBlockCount * BlockSize ||
        volume.start() > capacity || volume.length() > capacity - volume.start()) return {};
    return BlockVolume(*blocks, volume, Formats::AmigaDos, BlockSize);
}

class AmigaDosDocumentReader final : public SectorReader {
public:
    std::string_view id() const noexcept override { return CatalogConstants::Reader0; }
    std::span<const std::string_view> catalogFormatIds() const noexcept override { return CatalogConstants::Formats0; }
    bool canRead(const SectorImage& image, std::stop_token token) const override { return AmigaDos::canRead(image, token); }
    FileSystemVolume read(const SectorImage& image, std::stop_token token) const override { return AmigaDos::read(image, token); }
    bool canRead(const ImageDocument& document, const VolumeDescriptor& volume, std::stop_token token) const override {
        if (dynamic_cast<const SectorRepresentation*>(&document.representation()))
            return SectorReader::canRead(document, volume, token);
        const auto image = blockVolume(document, volume);
        if (!image) return false;
        const auto boot = image->block(0, token);
        if (!std::equal(DosSignature.begin(), DosSignature.end(), boot->data.begin())) return false;
        return AmigaDos::Internal::root(*image, token).has_value();
    }
    FileSystemVolume read(const ImageDocument& document, const VolumeDescriptor& volume, std::stop_token token) const override {
        if (const auto image = blockVolume(document, volume)) return read(*image, token);
        return SectorReader::read(document, volume, token);
    }
};
}

std::shared_ptr<const SectorReader> createAmigaDosDocumentReader() {
    return std::make_shared<AmigaDosDocumentReader>();
}
}
