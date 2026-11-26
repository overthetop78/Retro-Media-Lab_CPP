#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <QFileInfo>
#include <algorithm>
#include <limits>

namespace RetroMediaLab::Emulation::StorageRules {
namespace {
auto slotKey(const MediaSlot& slot) { return std::pair{slot.kind, slot.id}; }

QString normalizedExtension(QStringView extension) {
    while (extension.startsWith(u'.')) extension = extension.sliced(1);
    return extension.toString().toCaseFolded();
}
}

std::optional<qint64> hardDiskCapacity(const HardDiskImageGeometry& geometry) noexcept {
    auto capacity = geometry.cylinders;
    if (capacity <= 0) return {};
    for (int factor : {geometry.heads, geometry.sectorsPerTrack, geometry.sectorSize}) {
        if (factor <= 0 || capacity > std::numeric_limits<qint64>::max() / factor) return {};
        capacity *= factor;
    }
    return capacity;
}

bool supportsHardDiskCapacity(qint64 capacity, const HardDiskImageFormat& format) noexcept {
    return capacity > 0 && capacity >= format.minimumCapacity && capacity <= format.maximumCapacity &&
        (!format.allowedCapacities || format.allowedCapacities->contains(capacity)) &&
        std::ranges::any_of(format.logicalSectorSizes, [capacity](int size) { return size > 0 && capacity % size == 0; });
}

bool validHardDiskRequest(const QString& path, const HardDiskImageFormat& format, const HardDiskImageRequest& request) {
    const auto extension = QFileInfo(path).suffix();
    if (!format.canCreate || extension.isEmpty() || extension.toCaseFolded() != normalizedExtension(format.extension) ||
        !supportsHardDiskCapacity(request.capacity, format) ||
        (request.preallocate && !format.supportsFixedAllocation)) return false;

    if (!request.preparationId.isEmpty()) {
        const auto preparation = std::ranges::find_if(format.preparations,
            [&](const auto& candidate) { return candidate.choice.id == request.preparationId; });
        if (preparation == format.preparations.end() ||
            (!format.createImage && (format.mediaEngineFormatId.isEmpty() || preparation->fileSystemId.isEmpty())))
            return false;
    }
    if (format.fixedGeometry && request.geometry != format.fixedGeometry) return false;
    if (!request.geometry) return !format.supportsExplicitGeometry && format.mediaEngineFormatId.isEmpty();
    return hardDiskCapacity(*request.geometry) == request.capacity &&
        format.logicalSectorSizes.contains(request.geometry->sectorSize);
}

bool validSlots(const StorageSettings& settings, const QList<StorageDevice>& devices) {
    QSet<std::pair<MediaKind, QString>> available;
    QSet<std::pair<MediaKind, QString>> configured;
    for (const auto& device : devices) available.insert(slotKey(device.slot));
    for (const auto& slot : settings.configuredSlots) {
        const auto key = slotKey(slot);
        if (!available.contains(key) || configured.contains(key)) return false;
        configured.insert(key);
    }
    QSet<std::pair<MediaKind, QString>> occupied;
    for (const auto& mount : settings.mountedMedia) {
        const auto key = slotKey(mount.media.slot);
        if (occupied.contains(key)) return false;
        occupied.insert(key);
    }
    return true;
}

bool validMount(const StorageMount& mount, const QList<StorageDevice>& devices, const QList<MediaSlot>& configured) {
    const auto& media = mount.media;
    const auto key = slotKey(media.slot);
    if (media.path.trimmed().isEmpty() || media.slot.kind == MediaKind::MemoryCard ||
        (!mount.inserted && !supportsEjection(media.slot.kind)) ||
        (requiresReadOnly(media.slot.kind) && !media.readOnly) ||
        !std::ranges::any_of(configured, [&](const auto& slot) { return slotKey(slot) == key; })) return false;

    const StorageDevice* target = nullptr;
    for (const auto& device : devices) {
        if (slotKey(device.slot) != key) continue;
        if (target) return false;
        target = &device;
    }
    if (!target) return false;
    const auto extension = QFileInfo(media.path).suffix();
    return !extension.isEmpty() && std::ranges::any_of(target->slot.acceptedExtensions, [&](const auto& accepted) {
        return normalizedExtension(accepted) == extension.toCaseFolded();
    });
}

bool acceptsCardFile(const StorageDevice& device, const QString& path) {
    if (device.slot.kind != MediaKind::MemoryCard || device.usesDirectory || path.isEmpty()) return false;
    const auto extension = QFileInfo(path).suffix();
    return !extension.isEmpty() && std::ranges::any_of(device.slot.acceptedExtensions, [&](const auto& accepted) {
        return normalizedExtension(accepted) == extension.toCaseFolded();
    });
}

MemoryCardPathIssue validateMemoryCardPath(const StorageDevice& device, const QString& path) {
    if (path.isEmpty()) return MemoryCardPathIssue::None;
    const QFileInfo info(path);
    if (device.usesDirectory) return info.isDir() ? MemoryCardPathIssue::None : MemoryCardPathIssue::Missing;
    if (!info.isFile()) return MemoryCardPathIssue::Missing;
    if (!acceptsCardFile(device, path)) return MemoryCardPathIssue::InvalidFormat;
    if (device.fixedImageSizeBytes && info.size() != *device.fixedImageSizeBytes)
        return MemoryCardPathIssue::InvalidSize;
    return MemoryCardPathIssue::None;
}

QHash<MediaKind, bool> firstSlotActivityFromLeds(const QHash<qint32, bool>& leds,
        bool includeOpticalDisc) {
    constexpr qint32 FloppyLed = 0, TapeLed = 1, CartridgeLed = 2, OpticalDiscLed = 3;
    QHash<MediaKind, bool> activity{
        {MediaKind::Floppy, leds.value(FloppyLed)},
        {MediaKind::Tape, leds.value(TapeLed)},
        {MediaKind::Cartridge, leds.value(CartridgeLed)}
    };
    if (includeOpticalDisc) activity.insert(MediaKind::OpticalDisc, leds.value(OpticalDiscLed));
    return activity;
}

bool supportsEjection(MediaKind kind) noexcept {
    return kind == MediaKind::Floppy || kind == MediaKind::OpticalDisc ||
        kind == MediaKind::Cartridge || kind == MediaKind::Tape;
}

bool requiresReadOnly(MediaKind kind) noexcept {
    return kind == MediaKind::OpticalDisc || kind == MediaKind::Cartridge;
}

StorageSettings restrictAcceptedExtensions(StorageSettings settings, MediaKind kind,
    const std::optional<QSet<QString>>& allowed) {
    if (!allowed) return settings;
    QSet<QString> extensions;
    for (const auto& extension : *allowed) extensions.insert(normalizedExtension(extension));
    for (auto& device : settings.availableDevices) {
        if (device.slot.kind == kind)
            device.slot.acceptedExtensions.removeIf([&](const auto& extension) {
                return !extensions.contains(normalizedExtension(extension));
            });
    }
    return settings;
}
}
