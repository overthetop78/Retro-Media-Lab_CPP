#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <limits>

namespace RetroMediaLab::Emulation::StorageRules {
namespace {
auto slotKey(const MediaSlot& slot) { return std::pair{slot.kind, slot.id}; }

struct ProtocolMediaKind {
    MediaKind kind;
    int category, type;
    QLatin1StringView prefix;
};

// The C# category and type enums have different wire values for cartridges and tapes.
constexpr ProtocolMediaKind ProtocolKinds[] = {
    {MediaKind::Floppy, 0, 0, QLatin1StringView("Floppy")},
    {MediaKind::HardDisk, 1, 1, QLatin1StringView("HardDisk")},
    {MediaKind::OpticalDisc, 2, 2, QLatin1StringView("Cd")},
    {MediaKind::Cartridge, 3, 4, QLatin1StringView("Cartridge")},
    {MediaKind::Tape, 4, 5, QLatin1StringView("Tape")},
};
constexpr auto PathKey = QLatin1StringView("path");
constexpr auto SlotKey = QLatin1StringView("slot");
constexpr auto CategoryKey = QLatin1StringView("category");
constexpr auto IndexKey = QLatin1StringView("index");
constexpr auto TypeKey = QLatin1StringView("type");
constexpr auto ReadOnlyKey = QLatin1StringView("isReadOnly");
constexpr auto InsertedKey = QLatin1StringView("isInserted");
constexpr auto ParametersKey = QLatin1StringView("parameters");

bool validMedia(const StorageMount& mount) {
    const auto kind = mount.media.slot.kind;
    return !mount.media.path.trimmed().isEmpty() && !mount.media.slot.id.isEmpty() &&
        std::ranges::any_of(ProtocolKinds, [kind](const auto& entry) { return entry.kind == kind; }) &&
        (mount.inserted || supportsEjection(kind)) &&
        (mount.media.readOnly || !requiresReadOnly(kind));
}

QString normalizedExtension(QStringView extension) {
    while (extension.startsWith(u'.')) extension = extension.sliced(1);
    return extension.toString().toCaseFolded();
}
}

const QList<SettingsChoice>& standardDriveSpeedChoices() {
    static const QList<SettingsChoice> choices{
        {QStringLiteral("100"), {}, QStringLiteral("100 %")},
        {QStringLiteral("200"), {}, QStringLiteral("200 %")},
        {QStringLiteral("400"), {}, QStringLiteral("400 %")},
        {QStringLiteral("800"), {}, QStringLiteral("800 %")},
        {QStringLiteral("0"), QStringLiteral("Emulation.Value.Maximum"), {}},
    };
    return choices;
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
    if (!validMedia(mount) ||
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

bool validMounts(const QList<StorageMount>& mounts) {
    QSet<std::pair<MediaKind, QString>> occupied;
    for (const auto& mount : mounts) {
        const auto key = slotKey(mount.media.slot);
        if (!validMedia(mount) || occupied.contains(key)) return false;
        occupied.insert(key);
    }
    return true;
}

std::optional<QList<StorageMount>> replaceMount(const QList<StorageMount>& mounts, const StorageMount& replacement) {
    auto result = mounts;
    result.removeIf([&](const auto& mount) { return slotKey(mount.media.slot) == slotKey(replacement.media.slot); });
    result.append(replacement);
    if (!validMounts(result)) return {};
    return result;
}

std::optional<QList<StorageMount>> ejectMount(const QList<StorageMount>& mounts, const MediaSlot& slot) {
    auto result = mounts;
    const auto existing = std::ranges::find_if(result, [&](const auto& mount) {
        return slotKey(mount.media.slot) == slotKey(slot);
    });
    if (existing == result.end()) return result;
    existing->inserted = false;
    if (!validMounts(result)) return {};
    return result;
}

std::optional<QByteArray> serializeMounts(const QList<StorageMount>& mounts) {
    if (!validMounts(mounts)) return {};
    QJsonArray entries;
    for (const auto& mount : mounts) {
        const auto& media = mount.media;
        const auto kind = std::ranges::find(ProtocolKinds, media.slot.kind, &ProtocolMediaKind::kind);
        if (!media.slot.id.startsWith(kind->prefix)) return {};
        bool converted = false;
        const auto suffix = QStringView(media.slot.id).sliced(kind->prefix.size());
        const int index = suffix.toInt(&converted);
        if (!converted || index < 0 || suffix != QString::number(index)) return {};
        QJsonObject parameters;
        for (auto it = media.parameters.cbegin(); it != media.parameters.cend(); ++it)
            parameters.insert(it.key(), it.value());
        const QJsonObject slot{{CategoryKey, kind->category}, {IndexKey, index}};
        entries.append(QJsonObject{{PathKey, media.path}, {SlotKey, slot}, {TypeKey, kind->type},
            {ReadOnlyKey, media.readOnly}, {InsertedKey, mount.inserted},
            {ParametersKey, parameters.isEmpty() ? QJsonValue(QJsonValue::Null) : QJsonValue(parameters)}});
    }
    return QJsonDocument(entries).toJson(QJsonDocument::Compact);
}

std::optional<QList<StorageMount>> deserializeMounts(const QByteArray& payload) {
    const auto document = QJsonDocument::fromJson(payload);
    if (!document.isArray()) return {};
    QList<StorageMount> mounts;
    for (const auto& value : document.array()) {
        const auto entry = value.toObject();
        const auto slot = entry.value(SlotKey).toObject();
        constexpr int InvalidValue = -1;
        const auto kind = std::ranges::find(ProtocolKinds, slot.value(CategoryKey).toInt(InvalidValue),
            &ProtocolMediaKind::category);
        const int index = slot.value(IndexKey).toInt(InvalidValue);
        if (kind == std::end(ProtocolKinds) || index < 0 || entry.value(TypeKey).toInt(InvalidValue) != kind->type ||
            !entry.value(ReadOnlyKey).isBool() || !entry.value(InsertedKey).isBool()) return {};
        const auto data = entry.value(ParametersKey);
        if (!data.isUndefined() && !data.isNull() && !data.isObject()) return {};
        QHash<QString, QString> parameters;
        const auto fields = data.toObject();
        for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
            if (!it.value().isString()) return {};
            parameters.insert(it.key(), it.value().toString());
        }
        MediaSlot mediaSlot{QString(kind->prefix) + QString::number(index), {}, kind->kind};
        mounts.append(StorageMount{MountedMedia{std::move(mediaSlot), entry.value(PathKey).toString(),
            entry.value(ReadOnlyKey).toBool(), std::move(parameters)}, entry.value(InsertedKey).toBool()});
    }
    if (!validMounts(mounts)) return {};
    return mounts;
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
