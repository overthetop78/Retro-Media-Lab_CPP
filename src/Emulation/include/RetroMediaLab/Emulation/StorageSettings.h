#pragma once

#include "Host.h"
#include <functional>

class QIODevice;

namespace RetroMediaLab::Emulation {
enum class StorageConfigurationKind { None, FloppyDrive, HardDiskDrive, TapeDrive, CartridgeSlot, CompactDiscDrive, MemoryCard, ManagedStorage };

inline constexpr int DefaultHardDiskSectorSize = 512;
inline constexpr int DefaultHardDiskHeads = 1;
inline constexpr int DefaultHardDiskSectorsPerTrack = 32;

struct HardDiskImageGeometry {
    qint64 cylinders = 0;
    int heads = DefaultHardDiskHeads;
    int sectorsPerTrack = DefaultHardDiskSectorsPerTrack;
    int sectorSize = DefaultHardDiskSectorSize;
    bool operator==(const HardDiskImageGeometry&) const = default;
};

struct HardDiskImageRequest {
    qint64 capacity = 0;
    bool preallocate = false;
    QString preparationId;
    std::optional<HardDiskImageGeometry> geometry;
};

struct HardDiskImagePreparation {
    SettingsChoice choice;
    QString fileSystemId;
};

struct HardDiskImageFormat {
    QString id, extension, interfaceName, displayName, mediaEngineFormatId;
    qint64 minimumCapacity = DefaultHardDiskSectorSize;
    qint64 maximumCapacity = 0;
    qint64 defaultCapacity = 0;
    std::optional<QList<qint64>> allowedCapacities;
    QList<int> logicalSectorSizes{DefaultHardDiskSectorSize};
    int preferredHeads = DefaultHardDiskHeads;
    int preferredSectorsPerTrack = DefaultHardDiskSectorsPerTrack;
    std::optional<HardDiskImageGeometry> fixedGeometry;
    QList<HardDiskImagePreparation> preparations;
    bool supportsExplicitGeometry = false;
    bool supportsFixedAllocation = true;
    bool canCreate = true;
    std::function<void(QIODevice&)> initializeBlankImage;
    // Custom creators publish the entire image set without overwriting any destination.
    std::function<void(const QString&, const HardDiskImageRequest&, std::stop_token)> createImage;
};

struct FloppyCreationFormat {
    QString formatId, extension, displayName;
    qint64 capacity = 0;
    int tracks = 0, sides = 0;
    bool doubleDensity = false;
};

struct FloppyDriveModel {
    SettingsChoice choice;
    qint64 blankImageSize = 0;
    double diameterInches = 0;
    int tracks = 0, sides = 0;
    bool doubleDensity = false;
    QList<FloppyCreationFormat> creationFormats;
    QStringList acceptedExtensions;
    QHash<QString, QString> blankFormats;
};

struct FloppyDriveOptions {
    QList<FloppyDriveModel> models;
    QString imageDirectory, imageFilter, defaultExtension;
    bool canCreateBlankMedia = true;
    bool canConfigureBehavior = true;
    bool canRedirectWrites = true;
    bool canConfigureWriteProtection = true;
    std::optional<QList<SettingsChoice>> speedChoices;
};

struct StorageDialogPresentation {
    QString deviceResourceKey, imageResourceKey, useExistingResourceKey, createResourceKey, associatedResourceKey;
    QString fileNameResourceKey, destinationFolderResourceKey, deviceNameResourceKey, deviceIdentifierResourceKey;
    QString modelFieldResourceKey, useActionResourceKey, existingHintResourceKey;
    QString imageRequiredResourceKey, invalidSizeResourceKey, invalidFormatResourceKey;
};

struct StorageDevice {
    MediaSlot slot;
    bool removable = true;
    bool requiresMachineRecreation = false;
    QString displayLabel;
    std::optional<FloppyDriveOptions> floppyOptions;
    QList<SettingsChoice> interfaceChoices;
    QString imageDirectory;
    bool permanent = false;
    StorageConfigurationKind configurationKind = StorageConfigurationKind::None;
    QString optionResourceKey, optionDescriptionResourceKey;
    QStringList compactDiscSpeeds;
    QString compactDiscCacheResourceKey, compactDiscIgnoreErrorsResourceKey, modelResourceKey;
    std::optional<qint64> fixedImageSizeBytes;
    QString defaultImageFileName;
    std::optional<StorageDialogPresentation> dialogPresentation;
    QList<SettingsField> configurationFields;
    bool usesDirectory = false;
    QString defaultImageSubdirectory, typeResourceKey;
    QList<SettingsBlock> configurationBlocks;
    std::optional<FloppyDriveModel> floppyModel;
    std::function<QList<SettingsField>(const QHash<QString, QString>&)> describeCartridgeOptions;
    std::function<void(QIODevice&)> initializeBlankMemoryCard;
    QList<HardDiskImageFormat> hardDiskFormats;
};

struct FloppyDriveSettings {
    QString model, speed;
    bool writeProtected = false;
    bool redirectWrites = false;
    QHash<QString, QString> options;
};

struct CartridgeSlotSettings {
    bool optionEnabled = false;
    QHash<QString, QString> options;
};

struct CompactDiscDriveSettings {
    QString speed = QStringLiteral("1");
    bool cacheImage = false;
    bool ignoreErrors = false;
    QHash<QString, QString> options;
};

struct StorageDeviceSettings {
    MediaSlot slot;
    std::optional<FloppyDriveSettings> floppy;
    QString interfaceId;
    std::optional<CartridgeSlotSettings> cartridge;
    std::optional<CompactDiscDriveSettings> compactDisc;
    QString memoryCardPath;
    QHash<QString, QString> options;
};

struct StorageMount {
    MountedMedia media;
    bool inserted = true;
};

struct StorageSettings {
    QList<StorageDevice> availableDevices;
    QList<MediaSlot> configuredSlots;
    QList<StorageMount> mountedMedia;
    QList<StorageDeviceSettings> deviceSettings;
};

class IModuleStorageSettingsManager {
public:
    virtual ~IModuleStorageSettingsManager() = default;
    virtual StorageSettings describeStorageSettings(const Configuration&) const = 0;
    virtual Configuration applyStorageSettings(Configuration, const StorageSettings&) = 0;
};

namespace StorageRules {
// The external-core LED protocol covers the first slot of each returned kind.
RETROMEDIALAB_EMULATION_API QHash<MediaKind, bool> firstSlotActivityFromLeds(
    const QHash<qint32, bool>& leds, bool includeOpticalDisc);
RETROMEDIALAB_EMULATION_API std::optional<qint64> hardDiskCapacity(const HardDiskImageGeometry&) noexcept;
RETROMEDIALAB_EMULATION_API bool supportsHardDiskCapacity(qint64, const HardDiskImageFormat&) noexcept;
RETROMEDIALAB_EMULATION_API bool validHardDiskRequest(const QString&, const HardDiskImageFormat&,
    const HardDiskImageRequest&);
RETROMEDIALAB_EMULATION_API bool validSlots(const StorageSettings&, const QList<StorageDevice>&);
RETROMEDIALAB_EMULATION_API bool validMount(const StorageMount&, const QList<StorageDevice>&, const QList<MediaSlot>&);
RETROMEDIALAB_EMULATION_API bool acceptsCardFile(const StorageDevice&, const QString& path);
enum class MemoryCardPathIssue { None, Missing, InvalidFormat, InvalidSize };
RETROMEDIALAB_EMULATION_API MemoryCardPathIssue validateMemoryCardPath(const StorageDevice&, const QString& path);
RETROMEDIALAB_EMULATION_API bool supportsEjection(MediaKind) noexcept;
RETROMEDIALAB_EMULATION_API bool requiresReadOnly(MediaKind) noexcept;
RETROMEDIALAB_EMULATION_API StorageSettings restrictAcceptedExtensions(StorageSettings, MediaKind,
    const std::optional<QSet<QString>>& allowed);
}
}
