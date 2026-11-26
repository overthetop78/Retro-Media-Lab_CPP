#pragma once
#include <QDateTime>
#include "Sdk.h"
#include "EmulatorManager.h"
#include <QStringList>
#include "EmulationExport.h"

namespace RetroMediaLab::Emulation {
struct ControllerPortFeedback;
struct FirmwareCandidate;
struct StorageSettings;
struct InputSettings;
struct StorageDevice;
struct HardDiskImageFormat;
struct HardDiskImageRequest;
enum class ModuleIssue { InvalidManifest, IncompatibleApi, UnsafeLibraryPath, InvalidFactory, DuplicateId, LoadFailed };
struct ModuleDiagnostic {
    QString directory;
    ModuleIssue issue;
};
struct ModuleInfo {
    QString id;
    QString version;
    QString updateCatalogUrl;
    QString displayResourceKey;
    QString brandImagePath;
    QList<MachineDefinition> machines;
};

class RETROMEDIALAB_EMULATION_API Session final {
public:
    ~Session();
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    const Configuration& configuration() const noexcept;
    const QString& moduleId() const noexcept;
    IEmulatedMachine& machine() noexcept;
    quint64 runtimeRevision() const noexcept;
    void pumpAudio(bool audible);
    int volumePercent() const;
    bool audioVolumeIsZero() const;
    void setVolumePercent(int percent) noexcept;
    void stopAudio() noexcept;
    void setGraphicsApi(GraphicsApi);
    bool execute(LifecycleCommand, std::stop_token = {});
    std::optional<bool> controllerPointerMode() const;
    std::optional<bool> switchControllerPointer(std::stop_token = {});
    bool updateSettings(const QJsonObject&, std::stop_token = {});
    bool updateSettingsWithFeedback(const QJsonObject&, const QList<ControllerPortFeedback>&, std::stop_token = {});
    bool updateSettingsWithFirmware(const QJsonObject&, const QList<ControllerPortFeedback>&,
        const QList<FirmwareCandidate>&, std::stop_token = {});
    bool updateSettingsWithStorage(const QJsonObject&, const QList<ControllerPortFeedback>&,
        const QList<FirmwareCandidate>&, const StorageSettings&, std::stop_token = {});
    bool updateSettingsWithInput(const QJsonObject&, const QList<ControllerPortFeedback>&,
        const QList<FirmwareCandidate>&, const StorageSettings*, const InputSettings&, std::stop_token = {});
    bool updateSetting(const QString& id, const QString& value, std::stop_token = {});
    std::optional<StorageSettings> storageSettings() const;
    QList<MediaSlot> mediaSlots() const;
    const QList<MountedMedia>& mountedMedia() const noexcept;
    bool insertMedia(MountedMedia, std::stop_token = {});
    // Applies the adapter's optional preparation to a newly selected file.
    bool insertMediaFile(MountedMedia, std::stop_token = {});
    bool ejectMedia(const MediaSlot&, std::stop_token = {});
    bool recordTape(const MediaSlot&, const QString& path, const std::optional<TapePcmSettings>& = {},
        std::stop_token = {});
    static constexpr int FirstStateSlot = 1;
    static constexpr int StateSlotCount = 10;
    QDateTime stateSlotModified(int slot) const;
    bool saveStateSlot(int slot, std::stop_token = {});
    bool loadStateSlot(int slot, std::stop_token = {});
    bool canQuickSave() const;
    bool canQuickLoad() const;
    bool saveState(const QString& path, std::stop_token = {});
    bool loadState(const QString& path, std::stop_token = {});
    bool quickSave(std::stop_token = {});
    bool quickLoad(std::stop_token = {});
private:
    friend class Host;
    void stopForClose();
    void disposeRuntime(std::unique_ptr<IEmulatedMachine> stopped, std::stop_token);
    QString stateSlotPath(int slot) const;
    struct Data;
    explicit Session(std::unique_ptr<Data>);
    std::optional<Configuration> prepareMountedMedia(const QList<MountedMedia>&, const MediaSlot&,
        const QString& ejectedPath, std::stop_token);
    bool insertMediaImpl(MountedMedia, bool prepareFile, std::stop_token);
    void persistMountedMedia(const MediaSlot&, const QString& ejectedPath, std::stop_token);
    bool replaceRuntime(std::optional<QList<MountedMedia>>, std::optional<Configuration>, std::stop_token);
    bool requiresRecreation(const MediaSlot&) const;
    bool applySettings(const QJsonObject&, const QString& changedField, std::stop_token,
        const QList<ControllerPortFeedback>* controllerPorts = nullptr,
        const QList<FirmwareCandidate>* firmware = nullptr, const StorageSettings* storage = nullptr,
        const InputSettings* input = nullptr);
    std::unique_ptr<Data> data_;
};

class RETROMEDIALAB_EMULATION_API Host final {
public:
    Host(QString modulesDirectory, QString configurationDirectory);
    Host(QString modulesDirectory, QString configurationDirectory, QString stateDirectory);
    ~Host();
    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;
    void discover();
    void setStorageDirectories(QString storage, QString states);
    QList<ModuleInfo> modules() const;
    const QList<ModuleDiagnostic>& diagnostics() const noexcept;
    QString translate(const QString& moduleId, const QString& key, const QString& language) const;
    QList<Configuration> configurations(const QString& moduleId, std::stop_token = {}) const;
    Configuration newConfiguration(const QString& moduleId, const QString& machineId);
    Configuration configurationForGraphicsApi(const QString& moduleId, const Configuration&, GraphicsApi) const;
    QList<SettingsBlock> describe(const QString& moduleId, const Configuration&) const;
    QString audioOutputFieldId(const QString& moduleId, const Configuration&) const;
    QHash<QString, SettingsNumericRange> numericSettings(const QString& moduleId, const Configuration&) const;
    QHash<QString, VideoSettingsSection> videoSettingsSections(const QString& moduleId, const Configuration&) const;
    QHash<QString, SettingsChoicePresentation> choicePresentation(const QString& moduleId, const Configuration&) const;
    QSet<QString> settingsRefreshFields(const QString& moduleId, const Configuration&) const;
    SettingsDependencies settingsDependencies(const QString& moduleId, const Configuration&) const;
    QHash<QString, EditableNumericSetting> editableNumericSettings(const QString& moduleId, const Configuration&) const;
    QSet<QString> audioMixerBlocks(const QString& moduleId, const Configuration&) const;
    SettingsPresentation settingsPresentation(const QString& moduleId, const Configuration&) const;
    Configuration applySettings(const QString& moduleId, Configuration configuration, const QJsonObject& values);
    std::optional<EmulatorSelection> emulatorSelection(const QString& moduleId, const Configuration&, std::stop_token = {}) const;
    Configuration useEmulator(const QString& moduleId, Configuration, const QString& emulatorId, std::stop_token = {});
    QList<EmulatorRelease> findEmulatorReleases(const QString& moduleId, const Configuration&, std::stop_token = {}) const;
    QString installEmulator(const QString& moduleId, const Configuration&, const EmulatorRelease&,
        const EmulatorInstallProgress& = {}, std::stop_token = {});
    QString firmwareDirectory(const QString& moduleId, const QString& machineId) const;
    QList<FirmwareCandidate> scanFirmware(const QString& moduleId, const Configuration&, std::stop_token = {}) const;
    Configuration useFirmware(const QString& moduleId, Configuration, const FirmwareCandidate&);
    std::optional<StorageSettings> storageSettings(const QString& moduleId, const Configuration&) const;
    Configuration applyStorageSettings(const QString& moduleId, Configuration, const StorageSettings&);
    Configuration applyStorageSettings(const QString& moduleId, Configuration, const StorageSettings&, GraphicsApi);
    std::optional<InputSettings> inputSettings(const QString& moduleId, const Configuration&) const;
    Configuration applyInputSettings(const QString& moduleId, Configuration, const InputSettings&);
    bool saveInputSettings(const QString& moduleId, const Configuration&, std::stop_token = {});
    QList<ControllerPortFeedback> controllerFeedbackPorts(const QString& moduleId, const Configuration&) const;
    Configuration applySettingsWithFeedback(const QString& moduleId, Configuration configuration, const QJsonObject& values,
        const QList<ControllerPortFeedback>& controllerPorts);
    void saveConfiguration(const QString& moduleId, const Configuration&, std::stop_token = {});
    void deleteConfiguration(const QString& moduleId, const QUuid&, std::stop_token = {});
    std::shared_ptr<Session> open(const QString& moduleId, const Configuration&, std::stop_token = {});
    std::shared_ptr<Session> openWithGraphicsApi(const QString& moduleId, const Configuration&, GraphicsApi,
        std::stop_token = {});
    const QList<std::shared_ptr<Session>>& sessions() const noexcept;
    void close(const std::shared_ptr<Session>&);
    QStringList tapeRecordingExtensions(const MediaSlot&) const;
    void createBlankTape(const QString& path, std::optional<TapePcmSettings> = {}, std::stop_token = {}) const;
    void createBlankFloppy(const QString& path, const QString& formatId, qint64 rawSize,
        std::stop_token = {}) const;
    void createBlankMemoryCard(const QString& path, const StorageDevice&, std::stop_token = {}) const;
    void createHardDisk(const QString& path, const HardDiskImageFormat&, const HardDiskImageRequest&,
        std::stop_token = {}) const;
private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}
