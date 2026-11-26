#include <RetroMediaLab/Emulation/Host.h>
#include <RetroMediaLab/Emulation/Messages.h>
#include <RetroMediaLab/Emulation/Firmware.h>
#include <RetroMediaLab/Emulation/ControllerFeedback.h>
#include <RetroMediaLab/Emulation/SettingsRules.h>
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <RetroMediaLab/Emulation/InputSettings.h>
#include <RetroMediaLab/Emulation/Networks.h>
#include "MediaPreparation.h"
#include "AudioOutput.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLibrary>
#include <QMap>
#include <QRegularExpression>
#include "StagedFile.h"
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <QVersionNumber>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace {
void applyControllerFeedback(IEmulatedMachine& machine, const IEmulationModule& module, const Configuration& configuration) {
    auto* runtime = dynamic_cast<IMachineControllerFeedbackSettings*>(&machine);
    const auto* settings = dynamic_cast<const IModuleControllerFeedbackSettings*>(&module);
    if (!runtime) return;
    const auto* input = dynamic_cast<const IModuleInputSettingsManager*>(&module);
    if (!settings && !input) return;
    auto ports = settings ? settings->controllerFeedbackPorts(configuration) : QList<ControllerPortFeedback>{};
    if (input) {
        const auto modelSettings = input->describeInputSettings(configuration);
        for (const auto& port : modelSettings.controllerPorts) {
            auto feedback = controllerModelFeedback(port);
            if (!feedback) continue;
            ports.removeIf([&](const auto& legacy) { return legacy.number == port.number; });
            ports.append(std::move(*feedback));
        }
    }
    runtime->setControllerFeedbackBindings(createControllerFeedbackBindings(ports));
}

constexpr auto QuickStateDirectory = "Retro Media Lab/Emulation/States";
constexpr auto QuickStateFile = "%1-%2.gwas";
constexpr auto NumberedStateFile = "%1-%2.%3";
constexpr auto SlotStagingDirectory = ".rml-state-XXXXXX";
constexpr auto SessionDirectory = "Sessions";
constexpr auto RuntimeStateDirectory = "States";
constexpr auto ConvertedDirectory = "Converted";
constexpr auto Manifest = "module.json";
constexpr auto MaxManifestSize = 64 * 1024;
constexpr auto Schema = "schemaVersion";
constexpr int SchemaVersion = 2;
constexpr auto Id = "id";
constexpr auto Entry = "entryAssembly";
constexpr auto Version = "moduleVersion";
constexpr auto Minimum = "hostApiMinimum";
constexpr auto Maximum = "hostApiMaximum";
constexpr auto Updates = "updateCatalogUrl";
constexpr auto EmulatorRejected = "Emulation.Error.EmulatorRejected";

ModuleAudioOutputSelection audioOutputSelection(const IEmulationModule& module, const Configuration& configuration) {
    const auto* audio = dynamic_cast<const IModuleAudioOutputSettings*>(&module);
    return audio ? audio->audioOutputSelection(configuration) : ModuleAudioOutputSelection{};
}

bool preservesEmulator(IEmulationModule& module, const Configuration& configuration,
    const QList<Configuration>& saved, std::stop_token token) {
    auto* manager = dynamic_cast<IModuleEmulatorManager*>(&module);
    if (!manager) return !token.stop_requested();
    const auto selected = manager->emulatorInstallation(configuration, token).emulator.id;
    const auto existing = std::ranges::find(saved, configuration.machineId, &Configuration::machineId);
    return !selected.isEmpty() && (existing == saved.end() ||
        selected == manager->emulatorInstallation(*existing, token).emulator.id) && !token.stop_requested();
}

SettingsDependencies dependenciesFor(const IEmulationModule& module, const Configuration& config) {
    const auto* provider = dynamic_cast<const IModuleSettingsDependencies*>(&module);
    return provider ? provider->settingsDependencies(config) : SettingsDependencies{};
}

bool sameSlot(const MediaSlot& left, const MediaSlot& right) {
    return left.id == right.id && left.kind == right.kind;
}

Configuration applyStorageDraft(IModuleStorageSettingsManager& manager, Configuration configuration, StorageSettings settings) {
    if (!StorageRules::validSlots(settings, settings.availableDevices) ||
        !std::ranges::all_of(settings.mountedMedia, [&](const auto& mount) {
            return StorageRules::validMount(mount, settings.availableDevices, settings.configuredSlots);
        })) return {};
    const auto configurationId = configuration.id;
    const auto machineId = configuration.machineId;
    auto updated = manager.applyStorageSettings(std::move(configuration), settings);
    return updated.id == configurationId && updated.machineId == machineId ? std::move(updated) : Configuration{};
}

bool isPowered(MachineState state) {
    return state == MachineState::Running || state == MachineState::Paused;
}

Configuration runtimeConfiguration(const IEmulationModule& module, const Configuration& configuration, GraphicsApi api) {
    const auto* video = dynamic_cast<const IModuleVideoSettingsManager*>(&module);
    if (!video) return configuration;
    auto updated = video->applyVideoSettings(configuration, api);
    if (updated.id != configuration.id || updated.machineId != configuration.machineId)
        throw std::invalid_argument(EmulatorRejected);
    return updated;
}

bool applyRuntimeOptions(IEmulatedMachine& machine, const IEmulationModule& module,
    const Configuration& configuration, std::stop_token token) {
    const auto* configured = dynamic_cast<const IModuleRuntimeOptions*>(&module);
    auto* runtime = dynamic_cast<IMachineRuntimeOptions*>(&machine);
    if (!configured || !runtime) return true;
    const auto options = configured->runtimeOptions(configuration);
    for (const auto& option : runtime->availableOptions()) {
        if (token.stop_requested()) return false;
        const auto desired = options.constFind(option.id);
        if (!option.requiresRestart && desired != options.constEnd() && *desired != option.currentValue)
            runtime->setOption(option.id, *desired, token);
    }
    return true;
}

bool sameMedia(const MountedMedia& left, const MountedMedia& right) {
    return sameSlot(left.slot, right.slot) && left.path == right.path &&
        left.readOnly == right.readOnly && left.parameters == right.parameters;
}

QList<MountedMedia> matchDevices(const QList<MediaSlot>& devices, const QList<MountedMedia>& desired) {
    QList<MountedMedia> matched;
    for (const auto& media : desired) {
        const auto target = std::ranges::find_if(devices, [&](const auto& device) { return sameSlot(device, media.slot); });
        if (target == devices.end()) continue;
        auto copy = media;
        copy.slot = *target;
        matched.append(std::move(copy));
    }
    return matched;
}

QList<MountedMedia> configuredMounts(const IModuleStorageSettingsManager& storage, const Configuration& configuration) {
    const auto settings = storage.describeStorageSettings(configuration);
    QList<MountedMedia> desired;
    for (const auto& mount : settings.mountedMedia)
        if (mount.inserted) desired.append(mount.media);
    for (const auto& device : settings.deviceSettings) {
        if (device.slot.kind == MediaKind::MemoryCard && !device.memoryCardPath.isEmpty() &&
            std::ranges::any_of(settings.configuredSlots, [&](const auto& slot) { return sameSlot(slot, device.slot); }))
            desired.append({device.slot, device.memoryCardPath, false});
    }
    return matchDevices(settings.configuredSlots, desired);
}

bool mountBeforeStart(IEmulatedMachine& runtime, const QList<MountedMedia>& desired, std::stop_token token) {
    const auto initial = runtime.mountedMedia();
    for (const auto& media : initial) {
        if (token.stop_requested()) return false;
        if (!std::ranges::any_of(desired, [&](const auto& next) { return sameSlot(next.slot, media.slot); }) &&
            !runtime.eject(media.slot, token)) return false;
    }
    for (const auto& media : desired) {
        if (token.stop_requested()) return false;
        if (!std::ranges::any_of(initial, [&](const auto& old) { return sameMedia(old, media); }) &&
            !runtime.insert(media, token)) return false;
    }
    return true;
}

struct Module {
    explicit Module(QString path) : library(std::move(path)) {}
    ~Module() {
        if (instance) factory->destroy(instance);
        if (library.isLoaded()) library.unload();
    }
    QLibrary library;
    const ModuleFactory* factory = nullptr;
    IEmulationModule* instance = nullptr;
    ModuleInfo info;
};

std::optional<QVersionNumber> apiVersion(const QJsonObject& manifest, const char* key) {
    const auto text = manifest.value(QLatin1String(key)).toString();
    qsizetype suffix = {};
    const auto version = QVersionNumber::fromString(text, &suffix);
    if (suffix != text.size() || version.segmentCount() != 2) return {};
    return version;
}

std::optional<ModuleIssue> readModule(const QFileInfo& folder, const QString& configDirectory,
        std::shared_ptr<Module>& result) {
    const QDir directory(folder.absoluteFilePath());
    QFile file(directory.filePath(QLatin1String(Manifest)));
    if (!file.open(QIODevice::ReadOnly) || file.size() > MaxManifestSize) return ModuleIssue::InvalidManifest;
    const auto manifest = QJsonDocument::fromJson(file.read(MaxManifestSize)).object();
    const auto id = manifest.value(QLatin1String(Id)).toString();
    static const QRegularExpression validId(QStringLiteral("^[a-z0-9][a-z0-9_-]*$"));
    const auto version = manifest.value(QLatin1String(Version)).toString();
    const auto updateCatalog = manifest.value(QLatin1String(Updates)).toString();
    const QUrl updates(updateCatalog);
    if (manifest.value(QLatin1String(Schema)).toInt() != SchemaVersion || !validId.match(id).hasMatch()
            || id != folder.fileName() || version.isEmpty()
            || (!updateCatalog.isEmpty() && (updates.scheme() != QStringLiteral("https") || updates.host().isEmpty())))
        return ModuleIssue::InvalidManifest;

    const QVersionNumber host(HostApiMajor, HostApiMinor);
    const auto minimum = apiVersion(manifest, Minimum);
    const auto maximum = apiVersion(manifest, Maximum);
    if (!minimum || !maximum || host < *minimum || host > *maximum) return ModuleIssue::IncompatibleApi;

    const auto entry = manifest.value(QLatin1String(Entry)).toString();
    const QFileInfo library(directory.filePath(entry));
    if (entry.isEmpty() || entry != library.fileName() || !library.isFile() || library.isSymLink()
            || library.canonicalPath() != folder.canonicalFilePath() || !QLibrary::isLibrary(entry))
        return ModuleIssue::UnsafeLibraryPath;
    auto module = std::make_shared<Module>(library.absoluteFilePath());
    if (!module->library.load()) return ModuleIssue::LoadFailed;
    const auto factory = reinterpret_cast<ModuleFactoryEntry>(module->library.resolve(FactorySymbol));
    module->factory = factory ? factory() : nullptr;
    if (!module->factory || !module->factory->create || !module->factory->destroy) return ModuleIssue::InvalidFactory;
    // The 2.2 runtime services append fields after the 2.1 media service.
    constexpr int MinimumFactoryMinor = 1;
    if (module->factory->apiMajor != HostApiMajor || module->factory->apiMinor < MinimumFactoryMinor ||
        module->factory->apiMinor > HostApiMinor)
        return ModuleIssue::IncompatibleApi;
    module->instance = module->factory->create({directory.absolutePath(), QDir(configDirectory).filePath(id)});
    if (!module->instance || module->instance->id() != id) return ModuleIssue::InvalidFactory;
    auto machines = module->instance->machines();
    QSet<QString> machineIds;
    for (const auto& machine : machines) {
        if (machine.id.isEmpty() || machine.displayResourceKey.isEmpty() || machineIds.contains(machine.id))
            return ModuleIssue::InvalidFactory;
        machineIds.insert(machine.id);
    }
    module->info = {id, version, updateCatalog, module->instance->displayResourceKey(), module->instance->brandImagePath(), std::move(machines)};
    result = std::move(module);
    return {};
}
}

namespace {
class StoppedMachine final : public IEmulatedMachine {
public:
    explicit StoppedMachine(QList<MediaSlot> devices) : slots_(std::move(devices)) {}
    MachineState state() const override { return MachineState::Stopped; }
    void execute(LifecycleCommand, std::stop_token) override {}
    std::shared_ptr<const VideoFrame> latestFrame() const override { return {}; }
    double framesPerSecond() const override { return 0; }
    void setInput(const InputSnapshot&) override {}
    QList<MediaSlot> mediaSlots() const override { return slots_; }

private:
    QList<MediaSlot> slots_;
};
}

struct Session::Data {
    std::shared_ptr<Module> module;
    std::shared_ptr<MediaPreparation> media;
    RuntimeServices services;
    Configuration configuration;
    QString quickStatePath;
    std::unique_ptr<IEmulatedMachine> machine;
    QList<MountedMedia> mounted;
    bool recreateOnStart = false;
    quint64 runtimeRevision = 0;
    AudioOutput audio;
    std::optional<int> volumePercent;
    bool closeRequested = false;
    GraphicsApi graphicsApi = GraphicsApi::Automatic;
    std::optional<StorageSettings> storageDescription;

    void updateConfiguration(Configuration updated) {
        configuration = std::move(updated);
        storageDescription.reset();
    }
};

Session::Session(std::unique_ptr<Data> data) : data_(std::move(data)) {}
Session::~Session() {
    try { stopForClose(); }
    catch (...) {
        // The machine is still destroyed before the module library unloads.
    }
}

void Session::stopForClose() {
    if (data_->closeRequested) return;
    data_->closeRequested = true;
    stopAudio();
    data_->machine->execute(LifecycleCommand::Stop, {});
}
void Session::disposeRuntime(std::unique_ptr<IEmulatedMachine> stopped, std::stop_token token) {
    stopAudio();
    auto previous = std::exchange(data_->machine, std::move(stopped));
    data_->recreateOnStart = true;
    ++data_->runtimeRevision;
    previous->execute(LifecycleCommand::Stop, token);
}

const Configuration& Session::configuration() const noexcept { return data_->configuration; }
const QString& Session::moduleId() const noexcept { return data_->module->info.id; }
IEmulatedMachine& Session::machine() noexcept { return *data_->machine; }
quint64 Session::runtimeRevision() const noexcept { return data_->runtimeRevision; }
namespace {
constexpr int MaximumVolumePercent = 100;
}

int Session::volumePercent() const {
    if (data_->volumePercent) return *data_->volumePercent;
    const auto configured = audioOutputSelection(*data_->module->instance, data_->configuration).volume;
    return std::isfinite(configured)
        ? qRound(std::clamp(configured, 0.0f, 1.0f) * MaximumVolumePercent) : MaximumVolumePercent;
}

bool Session::audioVolumeIsZero() const {
    if (data_->volumePercent) return *data_->volumePercent == 0;
    const float configured = audioOutputSelection(*data_->module->instance, data_->configuration).volume;
    return std::isfinite(configured) && configured <= 0.0f;
}

void Session::setVolumePercent(int percent) noexcept {
    data_->volumePercent = std::clamp(percent, 0, MaximumVolumePercent);
}

void Session::pumpAudio(bool audible) {
    auto selection = audioOutputSelection(*data_->module->instance, data_->configuration);
    if (data_->volumePercent) selection.volume = float(*data_->volumePercent) / MaximumVolumePercent;
    data_->audio.pump(*data_->machine, audible, selection);
}
void Session::stopAudio() noexcept { data_->audio.stop(*data_->machine); }
void Session::setGraphicsApi(GraphicsApi api) {
    if (data_->graphicsApi == api) return;
    auto& module = *data_->module->instance;
    if (dynamic_cast<const IModuleVideoSettingsManager*>(&module)) {
        const auto configured = runtimeConfiguration(module, data_->configuration, api);
        if (const auto* storage = dynamic_cast<const IModuleStorageSettingsManager*>(&module))
            data_->mounted = configuredMounts(*storage, configured);
        data_->graphicsApi = api;
        data_->storageDescription.reset();
        data_->recreateOnStart = true;
        if (isPowered(data_->machine->state())) applyRuntimeOptions(*data_->machine, module, configured, {});
    } else {
        data_->graphicsApi = api;
    }
}

std::optional<StorageSettings> Session::storageSettings() const {
    const auto& module = *data_->module->instance;
    const auto* storage = dynamic_cast<const IModuleStorageSettingsManager*>(&module);
    if (!storage) return std::nullopt;
    if (!data_->storageDescription) {
        const auto configuration = runtimeConfiguration(module, data_->configuration, data_->graphicsApi);
        data_->storageDescription = storage->describeStorageSettings(configuration);
    }
    return data_->storageDescription;
}

QList<MediaSlot> Session::mediaSlots() const {
    const auto settings = storageSettings();
    if (!settings) return data_->machine->mediaSlots();

    QList<MediaSlot> devices;
    for (const auto& device : settings->availableDevices) {
        if (std::ranges::any_of(settings->configuredSlots, [&](const auto& slot) { return sameSlot(slot, device.slot); }))
            devices.append(device.slot);
    }
    return devices;
}

const QList<MountedMedia>& Session::mountedMedia() const noexcept { return data_->mounted; }

bool Session::requiresRecreation(const MediaSlot& slot) const {
    const auto settings = storageSettings();
    return settings && std::ranges::any_of(settings->availableDevices, [&](const auto& device) {
        return device.requiresMachineRecreation && sameSlot(device.slot, slot);
    });
}

bool Session::execute(LifecycleCommand command, std::stop_token token) try {
    if (token.stop_requested()) return false;
    const auto state = data_->machine->state();
    if (command == LifecycleCommand::HardReset && isPowered(state) &&
        std::ranges::any_of(data_->mounted, [this](const auto& mount) { return requiresRecreation(mount.slot); }))
        return replaceRuntime(data_->mounted, {}, token);
    if (command == LifecycleCommand::Stop) {
        if (state != MachineState::Created && state != MachineState::Stopped)
            disposeRuntime(std::make_unique<StoppedMachine>(data_->machine->mediaSlots()), token);
        return true;
    }
    if (command == LifecycleCommand::Start && !data_->recreateOnStart && !isPowered(state)) {
        auto stopped = std::make_unique<StoppedMachine>(data_->machine->mediaSlots());
        try {
            if (dynamic_cast<const IModuleStorageSettingsManager*>(data_->module->instance) &&
                !dynamic_cast<const IModuleMountedMedia*>(data_->module->instance) &&
                (!mountBeforeStart(*data_->machine, data_->mounted, token) || token.stop_requested())) {
                disposeRuntime(std::move(stopped), {});
                return false;
            }
            data_->machine->execute(command, token);
        } catch (...) {
            if (stopped) {
                try { disposeRuntime(std::move(stopped), {}); } catch (...) {}
            }
            throw;
        }
        return true;
    }
    if (command != LifecycleCommand::Start || !data_->recreateOnStart) {
        data_->machine->execute(command, token);
        if (command == LifecycleCommand::Pause && data_->machine->state() == MachineState::Paused) {
            stopAudio();
        }
        return true;
    }
    if (state != MachineState::Created && state != MachineState::Stopped) return false;
    return replaceRuntime(std::nullopt, {}, token);
} catch (...) {
    if (command != LifecycleCommand::Start) throw;
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::Machine,
        MessageCode::MachineStartFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::replaceRuntime(std::optional<QList<MountedMedia>> desired, std::optional<Configuration> configuration,
    std::stop_token token) {
    if (token.stop_requested()) return false;
    auto& module = *data_->module->instance;
    auto* mediaFactory = dynamic_cast<IModuleMountedMedia*>(&module);
    const bool wasPowered = isPowered(data_->machine->state());
    const auto& next = configuration ? *configuration : data_->configuration;
    const auto configured = runtimeConfiguration(module, next, data_->graphicsApi);
    const auto* storage = dynamic_cast<const IModuleStorageSettingsManager*>(&module);
    const auto mounts = desired ? std::move(*desired) : storage ? configuredMounts(*storage, configured) : data_->mounted;

    if (!dynamic_cast<StoppedMachine*>(data_->machine.get())) {
        disposeRuntime(std::make_unique<StoppedMachine>(data_->machine->mediaSlots()), token);
        if (token.stop_requested()) return false;
    }

    auto replacement = mediaFactory ? mediaFactory->createRuntimeWithMedia(configured, data_->services, mounts, token) :
        module.createRuntime(configured, data_->services, token);
    if (!replacement) return false;
    const auto stopReplacement = qScopeGuard([&] {
        if (!replacement) return;
        try { replacement->execute(LifecycleCommand::Stop, {}); } catch (...) {}
    });

    applyControllerFeedback(*replacement, module, next);
    auto compatible = matchDevices(replacement->mediaSlots(), mounts);
    if (token.stop_requested() || (wasPowered && compatible.size() != mounts.size()) ||
        (!mediaFactory && !mountBeforeStart(*replacement, compatible, token))) return false;
    replacement->execute(LifecycleCommand::Start, token);
    if (token.stop_requested() || !isPowered(replacement->state())) return false;
    if (configuration) module.saveConfiguration(*configuration, token);

    data_->machine = std::move(replacement);
    ++data_->runtimeRevision;
    data_->mounted = std::move(compatible);
    data_->storageDescription.reset();
    if (configuration) data_->updateConfiguration(std::move(*configuration));
    data_->recreateOnStart = false;
    return true;
}

std::optional<Configuration> Session::prepareMountedMedia(const QList<MountedMedia>& mounted,
    const MediaSlot& slot, const QString& ejectedPath, std::stop_token token) {
    auto& module = *data_->module->instance;
    auto* settingsManager = dynamic_cast<IModuleStorageSettingsManager*>(&module);
    auto* storage = dynamic_cast<IModuleMountedMedia*>(&module);
    if ((!settingsManager && !storage) || token.stop_requested()) return std::nullopt;
    const auto saved = module.loadConfigurations(token);
    if (token.stop_requested()) return std::nullopt;
    const auto& current = data_->configuration;
    const auto found = std::ranges::find(saved, current.id, &Configuration::id);
    const auto& latest = found == saved.end() ? current : *found;
    Configuration updated;
    if (settingsManager) {
        const auto configured = runtimeConfiguration(module, latest, data_->graphicsApi);
        auto settings = settingsManager->describeStorageSettings(configured);
        if (slot.kind == MediaKind::MemoryCard) {
            const auto device = std::ranges::find_if(settings.availableDevices,
                [&](const auto& item) { return sameSlot(item.slot, slot); });
            if (device == settings.availableDevices.cend() ||
                device->configurationKind != StorageConfigurationKind::MemoryCard ||
                !std::ranges::any_of(settings.configuredSlots, [&](const auto& item) { return sameSlot(item, slot); }))
                return std::nullopt;
            auto configured = std::ranges::find_if(settings.deviceSettings,
                [&](const auto& item) { return sameSlot(item.slot, slot); });
            if (ejectedPath.isNull()) {
                const auto inserted = std::ranges::find_if(mounted,
                    [&](const auto& item) { return sameSlot(item.slot, slot); });
                if (inserted == mounted.end() || StorageRules::validateMemoryCardPath(*device, inserted->path) !=
                    StorageRules::MemoryCardPathIssue::None) return std::nullopt;
                if (configured == settings.deviceSettings.end())
                    configured = settings.deviceSettings.insert(settings.deviceSettings.end(), StorageDeviceSettings{slot});
                configured->memoryCardPath = inserted->path;
            } else {
                if (configured == settings.deviceSettings.end() || configured->memoryCardPath != ejectedPath)
                    return std::nullopt;
                configured->memoryCardPath.clear();
            }
        } else {
            std::optional<QList<StorageMount>> next;
            if (ejectedPath.isNull()) {
                const auto inserted = std::ranges::find_if(mounted,
                    [&](const auto& item) { return sameSlot(item.slot, slot); });
                if (inserted == mounted.end()) return std::nullopt;
                StorageMount mount{*inserted, true};
                if (!StorageRules::validMount(mount, settings.availableDevices, settings.configuredSlots)) return std::nullopt;
                next = StorageRules::replaceMount(settings.mountedMedia, mount);
            } else {
                const auto existingMount = std::ranges::find_if(settings.mountedMedia,
                    [&](const auto& item) { return sameSlot(item.media.slot, slot); });
                if (existingMount == settings.mountedMedia.end() || existingMount->media.path != ejectedPath) return std::nullopt;
                next = StorageRules::ejectMount(settings.mountedMedia, slot);
            }
            if (!next) return std::nullopt;
            settings.mountedMedia = std::move(*next);
        }
        updated = applyStorageDraft(*settingsManager, latest, std::move(settings));
    } else {
        updated = storage->applyMountedMedia(latest, mounted);
    }
    if (updated.id != current.id || updated.machineId != current.machineId ||
        !preservesEmulator(module, updated, {latest}, token)) {
        if (token.stop_requested()) return std::nullopt;
        throw std::invalid_argument(EmulatorRejected);
    }
    return updated;
}

void Session::persistMountedMedia(const MediaSlot& slot, const QString& ejectedPath, std::stop_token token) {
    if (auto configuration = prepareMountedMedia(data_->mounted, slot, ejectedPath, token)) {
        data_->module->instance->saveConfiguration(*configuration, token);
        data_->updateConfiguration(std::move(*configuration));
    }
}

bool Session::insertMedia(MountedMedia media, std::stop_token token) {
    return insertMediaImpl(std::move(media), false, token);
}

bool Session::insertMediaFile(MountedMedia media, std::stop_token token) {
    return insertMediaImpl(std::move(media), true, token);
}

bool Session::insertMediaImpl(MountedMedia media, bool prepareFile, std::stop_token token) try {
    if (token.stop_requested() || media.path.isEmpty()) return false;
    const auto devices = mediaSlots();
    const auto slot = std::ranges::find_if(devices, [&](const auto& entry) { return sameSlot(entry, media.slot); });
    if (slot == devices.end()) return false;
    media.slot = *slot;
    if (prepareFile) {
        const auto* adapter = dynamic_cast<const IModuleMediaPreparation*>(data_->module->instance);
        if (adapter) {
            media = adapter->prepareMedia(data_->configuration, data_->services, media, token);
            if (token.stop_requested() || media.path.isEmpty() || !sameSlot(*slot, media.slot)) return false;
            media.slot = *slot;
        }
    }
    const bool powered = isPowered(data_->machine->state());
    const bool recreate = requiresRecreation(*slot);
    const bool card = slot->kind == MediaKind::MemoryCard &&
        dynamic_cast<IModuleStorageSettingsManager*>(data_->module->instance);
    if (card || (powered && recreate)) {
        auto desired = data_->mounted;
        desired.removeIf([&](const auto& entry) { return sameSlot(entry.slot, *slot); });
        desired.append(media);
        auto configuration = prepareMountedMedia(desired, *slot, {}, token);
        if (!configuration) return false;
        if (powered && recreate) return replaceRuntime(desired, std::move(configuration), token);
        if (powered && !data_->machine->insert(media, token)) return false;
        data_->module->instance->saveConfiguration(*configuration, token);
        data_->updateConfiguration(std::move(*configuration));
        data_->mounted = std::move(desired);
        if (!powered) data_->recreateOnStart = true;
        return true;
    }
    if (isPowered(data_->machine->state())) {
        if (!data_->machine->insert(media, token)) return false;
    } else {
        data_->recreateOnStart = true;
    }
    auto& mounted = data_->mounted;
    const auto existing = std::ranges::find_if(mounted, [&](const auto& entry) { return sameSlot(entry.slot, media.slot); });
    if (existing == mounted.end()) mounted.append(std::move(media));
    else *existing = std::move(media);
    persistMountedMedia(*slot, {}, token);
    return true;
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::Media,
        MessageCode::MediaOperationFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::ejectMedia(const MediaSlot& slot, std::stop_token token) try {
    if (token.stop_requested()) return false;
    auto& mounted = data_->mounted;
    const auto existing = std::ranges::find_if(mounted, [&](const auto& media) { return sameSlot(media.slot, slot); });
    if (existing == mounted.end()) return false;
    const auto removed = *existing;
    if (isPowered(data_->machine->state()) && requiresRecreation(slot)) {
        auto desired = mounted;
        desired.removeIf([&](const auto& media) { return sameSlot(media.slot, slot); });
        auto configuration = prepareMountedMedia(desired, removed.slot, removed.path, token);
        return configuration && replaceRuntime(desired, std::move(configuration), token);
    }
    if (isPowered(data_->machine->state())) {
        if (!data_->machine->eject(existing->slot, token)) return false;
    } else {
        data_->recreateOnStart = true;
    }
    mounted.erase(existing);
    persistMountedMedia(removed.slot, removed.path, token);
    return true;
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::Media,
        MessageCode::MediaOperationFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::recordTape(const MediaSlot& slot, const QString& path, const std::optional<TapePcmSettings>& pcm,
    std::stop_token token) try {
    if (token.stop_requested() || slot.kind != MediaKind::Tape || path.isEmpty()) return false;
    auto& runtime = *data_->machine;
    if (!isPowered(runtime.state())) return false;
    const auto transport = runtime.tapeTransportStatus();
    if (!transport.availableCommands.contains(TapeCommand::Record) || transport.state == TapeState::Recording ||
        (transport.state == TapeState::Paused && transport.activeOperation == TapeCommand::Record)) return false;

    const auto configured = mediaSlots();
    const auto device = std::ranges::find_if(configured, [&](const auto& candidate) { return sameSlot(candidate, slot); });
    if (device == configured.end()) return false;
    const auto liveDevices = runtime.mediaSlots();
    const auto live = std::ranges::find_if(liveDevices, [&](const auto& candidate) { return sameSlot(candidate, slot); });
    if (live == liveDevices.end()) return false;

    QFileInfo file(path);
    const auto suffix = QLatin1Char('.') + file.suffix();
    if (!data_->media->tapeRecordingExtensions(*device).contains(suffix, Qt::CaseInsensitive)) return false;
    if (transport.availableCommands.contains(TapeCommand::Stop) && !runtime.tape(TapeCommand::Stop, token)) return false;
    if (runtime.hasUnsavedChanges(*live, token) && !runtime.saveChanges(*live, token)) return false;
    if (token.stop_requested()) return false;

    const auto target = file.absoluteFilePath();
    data_->media->createBlankTape(target, pcm, token);
    const auto current = std::ranges::find_if(data_->mounted, [&](const auto& media) { return sameSlot(media.slot, slot); });
    const auto parameters = pcm ? pcm->parameters() : QHash<QString, QString>{};
    const bool sameMount = current != data_->mounted.end() && QFileInfo(current->path) == file;
    const bool sameProfile = sameMount && std::ranges::all_of(parameters.asKeyValueRange(), [&](const auto& entry) {
        return current->parameters.value(entry.first) == entry.second;
    });
    if (!sameProfile) {
        file.refresh();
        if (!insertMedia({*device, target, !file.isWritable(), parameters}, token)) return false;
    }
    return data_->machine->tape(TapeCommand::Record, token);
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::Media,
        MessageCode::MediaOperationFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::updateSettings(const QJsonObject& changes, std::stop_token token) {
    return applySettings(changes, {}, token);
}

bool Session::updateSettingsWithFeedback(const QJsonObject& changes, const QList<ControllerPortFeedback>& ports, std::stop_token token) {
    return applySettings(changes, {}, token, &ports);
}

bool Session::updateSettingsWithFirmware(const QJsonObject& changes, const QList<ControllerPortFeedback>& ports,
    const QList<FirmwareCandidate>& firmware, std::stop_token token) {
    return applySettings(changes, {}, token, &ports, &firmware);
}

bool Session::updateSettingsWithStorage(const QJsonObject& changes, const QList<ControllerPortFeedback>& ports,
    const QList<FirmwareCandidate>& firmware, const StorageSettings& storage, std::stop_token token) {
    return applySettings(changes, {}, token, &ports, &firmware, &storage);
}

bool Session::updateSettingsWithInput(const QJsonObject& changes, const QList<ControllerPortFeedback>& ports,
    const QList<FirmwareCandidate>& firmware, const StorageSettings* storage, const InputSettings& input, std::stop_token token) {
    return applySettings(changes, {}, token, &ports, &firmware, storage, &input);
}

bool Session::updateSetting(const QString& id, const QString& value, std::stop_token token) {
    if (id.isEmpty()) return false;
    return applySettings({{id, value}}, id, token);
}

bool Session::applySettings(const QJsonObject& changes, const QString& changedField, std::stop_token token,
    const QList<ControllerPortFeedback>* controllerPorts, const QList<FirmwareCandidate>* firmware,
    const StorageSettings* storage, const InputSettings* input) {
    if (token.stop_requested()) return false;
    const bool feedbackChanged = controllerPorts && !controllerPorts->isEmpty();
    const bool firmwareChanged = firmware && !firmware->isEmpty();
    if (changes.isEmpty() && !feedbackChanged && !firmwareChanged && !storage && !input) return true;
    auto& module = *data_->module->instance;
    auto* feedback = feedbackChanged ? dynamic_cast<IModuleControllerFeedbackSettings*>(&module) : nullptr;
    if (feedbackChanged && !feedback) return false;
    auto* firmwareManager = firmwareChanged ? dynamic_cast<IModuleFirmwareManager*>(&module) : nullptr;
    if (firmwareChanged && !firmwareManager) return false;
    auto* storageManager = dynamic_cast<IModuleStorageSettingsManager*>(&module);
    if (storage && !storageManager) return false;
    auto* inputManager = input ? dynamic_cast<IModuleInputSettingsManager*>(&module) : nullptr;
    if (input && !inputManager) return false;
    const auto saved = module.loadConfigurations(token);
    if (token.stop_requested()) return false;
    const auto& current = data_->configuration;
    const auto found = std::ranges::find(saved, current.id, &Configuration::id);
    const auto& latest = found == saved.end() ? current : *found;
    auto patch = changes;
    if (!changedField.isEmpty()) {
        const auto described = runtimeConfiguration(module, latest, data_->graphicsApi);
        const auto dependencies = dependenciesFor(module, described);
        if (!dependencies.rules.isEmpty()) {
            const auto blocks = module.describe(described.machineId, &described);
            patch = SettingsRules::changesFor(dependencies, SettingsRules::values(blocks), changedField,
                changes.value(changedField).toString());
        }
    }
    auto updated = patch.isEmpty() ? latest : module.applySettings(latest, patch);
    if (firmwareManager) {
        for (const auto& candidate : *firmware) {
            if (token.stop_requested() || candidate.compatibility == FirmwareCompatibility::Incompatible) return false;
            updated = firmwareManager->useFirmware(std::move(updated), candidate);
        }
    }
    if (feedback) updated = feedback->applyControllerFeedbackSettings(std::move(updated), *controllerPorts);
    if (storage) {
        auto draft = *storage;
        const auto configured = runtimeConfiguration(module, updated, data_->graphicsApi);
        draft.availableDevices = storageManager->describeStorageSettings(configured).availableDevices;
        updated = applyStorageDraft(*storageManager, std::move(updated), std::move(draft));
    }
    if (inputManager) updated = inputManager->applyInputSettings(std::move(updated), *input);
    if (updated.id != current.id || updated.machineId != current.machineId ||
        !preservesEmulator(module, updated, {latest}, token)) return false;
    const auto configured = runtimeConfiguration(module, updated, data_->graphicsApi);
    std::optional<QList<MountedMedia>> mounts;
    if (storageManager) mounts = configuredMounts(*storageManager, configured);
    module.saveConfiguration(updated, token);
    data_->updateConfiguration(std::move(updated));
    if (mounts) data_->mounted = std::move(*mounts);
    data_->recreateOnStart = true;
    if (inputManager) inputManager->saveInputSettings(data_->configuration, token);
    if (feedback || inputManager) applyControllerFeedback(*data_->machine, module, data_->configuration);

    const auto state = data_->machine->state();
    if (!isPowered(state)) return true;
    if (token.stop_requested()) return false;
    if (auto* bindings = dynamic_cast<IMachineInputBindings*>(data_->machine.get()))
        bindings->updateInputBindings(data_->configuration);
    return applyRuntimeOptions(*data_->machine, module, configured, token);
}

std::optional<bool> Session::controllerPointerMode() const {
    const auto* input = dynamic_cast<const IMachineControllerPointerSwitch*>(data_->machine.get());
    if (!input || !input->supportsControllerPointerSwitch()) return std::nullopt;
    return input->controllerPointerMode();
}

std::optional<bool> Session::switchControllerPointer(std::stop_token token) {
    if (token.stop_requested() || !isPowered(data_->machine->state())) return std::nullopt;
    auto* input = dynamic_cast<IMachineControllerPointerSwitch*>(data_->machine.get());
    if (!input || !input->supportsControllerPointerSwitch()) return std::nullopt;
    return input->switchControllerPointer(token);
}

bool Session::canQuickSave() const {
    const auto state = data_->machine->state();
    return isPowered(state) && data_->machine->supportsSavedStates();
}

QString Session::stateSlotPath(int slot) const {
    if (slot < FirstStateSlot || slot > StateSlotCount) return {};
    if (slot == FirstStateSlot) return data_->quickStatePath;
    const QFileInfo first(data_->quickStatePath);
    return QDir(first.absolutePath()).filePath(QString::fromLatin1(NumberedStateFile)
        .arg(first.completeBaseName()).arg(slot).arg(first.suffix()));
}

QDateTime Session::stateSlotModified(int slot) const {
    const QFileInfo file(stateSlotPath(slot));
    return file.isFile() ? file.lastModified() : QDateTime{};
}

bool Session::canQuickLoad() const {
    return canQuickSave() && stateSlotModified(FirstStateSlot).isValid();
}

bool Session::saveState(const QString& path, std::stop_token token) try {
    if (token.stop_requested() || path.isEmpty()) return false;
    return data_->machine->saveState(path, token);
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::SavedState,
        MessageCode::SavedStateOperationFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::loadState(const QString& path, std::stop_token token) try {
    if (token.stop_requested() || path.isEmpty()) return false;
    return data_->machine->loadState(path, token);
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::SavedState,
        MessageCode::SavedStateOperationFailed, {moduleId(), configuration().machineId, {}, {}}));
}

bool Session::saveStateSlot(int slot, std::stop_token token) {
    const auto path = stateSlotPath(slot);
    if (token.stop_requested() || path.isEmpty() || !canQuickSave()) return false;
    const QFileInfo target(path);
    const QDir directory(target.absolutePath());
    if (!QDir().mkpath(directory.absolutePath())) return false;
    QTemporaryDir staging(directory.filePath(QLatin1String(SlotStagingDirectory)));
    if (!staging.isValid()) return false;

    const auto stagedPath = staging.filePath(target.fileName());
    if (!saveState(stagedPath, token) || token.stop_requested()) return false;
    const QFileInfo staged(stagedPath);
    if (!staged.isFile() || staged.isSymLink() || token.stop_requested()) return false;
    return !replaceStagedFile(stagedPath, target.absoluteFilePath());
}

bool Session::loadStateSlot(int slot, std::stop_token token) {
    if (token.stop_requested() || !canQuickSave()) return false;
    const auto path = stateSlotPath(slot);
    return QFileInfo(path).isFile() && loadState(path, token);
}

bool Session::quickSave(std::stop_token token) {
    return saveStateSlot(FirstStateSlot, token);
}

bool Session::quickLoad(std::stop_token token) {
    return loadStateSlot(FirstStateSlot, token);
}

struct Host::Data {
    QString modulesDirectory;
    QString configurationDirectory;
    QString stateDirectory, storageDirectory;
    std::shared_ptr<MediaPreparation> media = std::make_shared<MediaPreparation>();
    QMap<QString, std::shared_ptr<Module>> modules;
    QList<ModuleDiagnostic> diagnostics;
    QList<std::shared_ptr<Session>> sessions;
};

Host::Host(QString modules, QString configurations) : Host(std::move(modules), configurations,
    QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(QLatin1String(QuickStateDirectory))) {}

Host::Host(QString modules, QString configurations, QString states) : data_(std::make_unique<Data>()) {
    data_->modulesDirectory = std::move(modules);
    data_->configurationDirectory = std::move(configurations);
    data_->storageDirectory = QFileInfo(states).absolutePath();
    data_->stateDirectory = std::move(states);
}
Host::~Host() {
    const auto sessions = std::move(data_->sessions);
    for (const auto& session : sessions) {
        try { session->stopForClose(); } catch (...) {}
    }
}

void Host::setStorageDirectories(QString storage, QString states) {
    data_->storageDirectory = std::move(storage);
    data_->stateDirectory = std::move(states);
}

void Host::discover() {
    const QDir directory(data_->modulesDirectory);
    QMap<QString, std::shared_ptr<Module>> discovered;
    QList<ModuleDiagnostic> issues;
    for (const auto& folder : directory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name)) {
        if (!QFileInfo::exists(QDir(folder.absoluteFilePath()).filePath(QLatin1String(Manifest)))) continue;
        const auto existing = data_->modules.value(folder.fileName());
        if (existing) {
            discovered.insert(folder.fileName(), existing);
            continue;
        }
        std::shared_ptr<Module> module;
        try {
            if (const auto issue = readModule(folder, data_->configurationDirectory, module)) {
                issues.push_back({folder.fileName(), *issue});
                continue;
            }
        } catch (...) {
            issues.push_back({folder.fileName(), ModuleIssue::LoadFailed});
            continue;
        }
        if (discovered.contains(module->info.id)) issues.push_back({folder.fileName(), ModuleIssue::DuplicateId});
        else discovered.insert(module->info.id, module);
    }
    data_->modules.swap(discovered);
    data_->diagnostics.swap(issues);
    for (auto it = discovered.cbegin(); it != discovered.cend(); ++it)
        if (!data_->modules.contains(it.key()) && dynamic_cast<IEmulationNetworkManager*>(it.value()->instance))
            networks().setConfigurations(it.key(), {});
}

QList<ModuleInfo> Host::modules() const {
    QList<ModuleInfo> result;
    for (const auto& module : data_->modules) result.push_back(module->info);
    return result;
}
const QList<ModuleDiagnostic>& Host::diagnostics() const noexcept { return data_->diagnostics; }

QStringList Host::tapeRecordingExtensions(const MediaSlot& slot) const {
    return data_->media->tapeRecordingExtensions(slot);
}
void Host::createBlankTape(const QString& path, std::optional<TapePcmSettings> pcm, std::stop_token token) const {
    data_->media->createBlankTape(path, pcm, token);
}

namespace {
constexpr auto ImageStagingDirectory = ".rml-image-XXXXXX";
constexpr auto ImageCreationError = "Emulation.Error.EmulatorRejected";
constexpr auto ImageAlreadyExists = "Emulation.Storage.Media.AlreadyExists";

void publishNewImage(const QString& path, std::stop_token token, const auto& write) {
    const QFileInfo target(path);
    if (target.exists() || target.isSymLink()) throw std::runtime_error(ImageAlreadyExists);
    if (token.stop_requested() || target.fileName().isEmpty()) throw std::runtime_error(ImageCreationError);
    QTemporaryDir staging(target.dir().filePath(QLatin1String(ImageStagingDirectory)));
    if (!staging.isValid()) throw std::runtime_error(ImageCreationError);
    const auto stagingPath = staging.filePath(target.fileName());
    write(stagingPath);
    if (token.stop_requested() || !QFile::rename(stagingPath, target.absoluteFilePath()))
        throw std::runtime_error(ImageCreationError);
}

void writeFixedSizeImage(const QString& path, qint64 size, const std::function<void(QIODevice&)>& initialize = {},
        bool preallocate = false, std::stop_token token = {}) {
    constexpr qsizetype AllocationChunkSize = 1024 * 1024;
    QFile image(path);
    if (size < 0 || !image.open(QIODevice::ReadWrite | QIODevice::NewOnly))
        throw std::runtime_error(ImageCreationError);
    if (preallocate) {
        const QByteArray block(AllocationChunkSize, '\0');
        for (qint64 remaining = size; remaining > 0;) {
            const auto count = std::min<qint64>(remaining, block.size());
            if (token.stop_requested() || image.write(block.constData(), count) != count)
                throw std::runtime_error(ImageCreationError);
            remaining -= count;
        }
        if (!image.seek(0)) throw std::runtime_error(ImageCreationError);
    } else if (!image.resize(size)) {
        throw std::runtime_error(ImageCreationError);
    }
    if (initialize) initialize(image);
    if (image.size() != size || image.error() != QFileDevice::NoError || !image.flush())
        throw std::runtime_error(ImageCreationError);
}
}

void Host::createBlankFloppy(const QString& path, const QString& formatId, qint64 rawSize,
        std::stop_token token) const {
    publishNewImage(path, token, [&](const QString& stagingPath) {
        if (formatId.isEmpty()) writeFixedSizeImage(stagingPath, rawSize);
        else data_->media->createBlankDisk(stagingPath, formatId, token);
    });
}

void Host::createBlankMemoryCard(const QString& path, const StorageDevice& device, std::stop_token token) const {
    if (device.slot.kind != MediaKind::MemoryCard || device.usesDirectory ||
        !device.fixedImageSizeBytes || *device.fixedImageSizeBytes <= 0 ||
        !StorageRules::acceptsCardFile(device, path))
        throw std::runtime_error(ImageCreationError);
    publishNewImage(path, token, [&](const QString& stagingPath) {
        writeFixedSizeImage(stagingPath, *device.fixedImageSizeBytes, device.initializeBlankMemoryCard);
    });
}

void Host::createHardDisk(const QString& path, const HardDiskImageFormat& format, const HardDiskImageRequest& request,
        std::stop_token token) const {
    constexpr auto InvalidDisk = "Emulation.Storage.Disk.InvalidSize";
    if (token.stop_requested() || !StorageRules::validHardDiskRequest(path, format, request))
        throw std::runtime_error(InvalidDisk);
    const QFileInfo target(path);
    if (target.exists() || target.isSymLink()) throw std::runtime_error(ImageAlreadyExists);
    if (format.createImage) {
        format.createImage(target.absoluteFilePath(), request, token);
    } else if (!format.mediaEngineFormatId.isEmpty()) {
        const auto preparation = std::ranges::find_if(format.preparations,
            [&](const auto& entry) { return entry.choice.id == request.preparationId; });
        const auto fileSystem = request.preparationId.isEmpty() ? QString{} : preparation->fileSystemId;
        data_->media->createHardDisk(target.absoluteFilePath(), format.mediaEngineFormatId, *request.geometry,
            fileSystem, request.preallocate, token);
    } else {
        publishNewImage(path, token, [&](const QString& stagingPath) {
            writeFixedSizeImage(stagingPath, request.capacity, format.initializeBlankImage, request.preallocate, token);
        });
    }
}

QString Host::translate(const QString& id, const QString& key, const QString& language) const {
    const auto module = data_->modules.value(id);
    if (!module) return key;
    const auto localized = module->instance->translate(key, language);
    return localized.isEmpty() ? key : localized;
}
QList<Configuration> Host::configurations(const QString& id, std::stop_token token) const {
    const auto module = data_->modules.value(id);
    if (!module) return {};
    auto configurations = module->instance->loadConfigurations(token);
    if (const auto* network = dynamic_cast<const IEmulationNetworkManager*>(module->instance)) {
        QList<NetworkEndpoint> endpoints;
        for (const auto& configuration : configurations) {
            if (token.stop_requested()) return configurations;
            endpoints.append(network->describeNetworks(configuration));
        }
        if (!token.stop_requested()) networks().setConfigurations(id, std::move(endpoints));
    }
    return configurations;
}
Configuration Host::newConfiguration(const QString& id, const QString& machineId) {
    const auto module = data_->modules.value(id);
    if (!module || std::none_of(module->info.machines.begin(), module->info.machines.end(),
        [&](const auto& machine) { return machine.id == machineId; })) return {};
    return module->instance->createConfiguration(machineId);
}
Configuration Host::configurationForGraphicsApi(const QString& id, const Configuration& configuration, GraphicsApi api) const {
    const auto module = data_->modules.value(id);
    return module ? runtimeConfiguration(*module->instance, configuration, api) : configuration;
}

QList<SettingsBlock> Host::describe(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    if (!module) return {};
    auto blocks = module->instance->describe(configuration.machineId, &configuration);
    const auto selection = audioOutputSelection(*module->instance, configuration);
    if (selection.fieldId.isEmpty()) return blocks;
    const auto devices = AudioOutput::deviceChoices();
    for (auto& block : blocks) {
        for (auto& field : block.fields) {
            if (field.id != selection.fieldId || field.editor != SettingsEditor::Choice) continue;
            for (const auto& device : devices) {
                if (std::ranges::find(field.choices, device.id, &SettingsChoice::id) == field.choices.end())
                    field.choices.append(device);
            }
        }
    }
    return blocks;
}
QString Host::audioOutputFieldId(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    return module ? audioOutputSelection(*module->instance, configuration).fieldId : QString{};
}
QHash<QString, SettingsNumericRange> Host::numericSettings(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* numeric = module ? dynamic_cast<const IModuleNumericSettings*>(module->instance) : nullptr;
    return numeric ? numeric->numericSettings(configuration) : QHash<QString, SettingsNumericRange>{};
}
QHash<QString, EditableNumericSetting> Host::editableNumericSettings(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* editor = module ? dynamic_cast<const IModuleEditableNumericSettings*>(module->instance) : nullptr;
    return editor ? editor->editableNumericSettings(configuration) : QHash<QString, EditableNumericSetting>{};
}

QSet<QString> Host::audioMixerBlocks(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* mixer = module ? dynamic_cast<const IModuleAudioMixerBlocks*>(module->instance) : nullptr;
    return mixer ? mixer->audioMixerBlocks(configuration) : QSet<QString>{};
}

SettingsPresentation Host::settingsPresentation(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* presentation = module ? dynamic_cast<const IModuleSettingsPresentation*>(module->instance) : nullptr;
    return presentation ? presentation->settingsPresentation(configuration) : SettingsPresentation{};
}

QHash<QString, VideoSettingsSection> Host::videoSettingsSections(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* provider = module ? dynamic_cast<const IModuleVideoSettingsSections*>(module->instance) : nullptr;
    return provider ? provider->videoSettingsSections(configuration) : QHash<QString, VideoSettingsSection>{};
}

QHash<QString, SettingsChoicePresentation> Host::choicePresentation(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* provider = module ? dynamic_cast<const IModuleChoicePresentation*>(module->instance) : nullptr;
    return provider ? provider->choicePresentation(configuration) : QHash<QString, SettingsChoicePresentation>{};
}

QSet<QString> Host::settingsRefreshFields(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* refresh = module ? dynamic_cast<const IModuleSettingsRefresh*>(module->instance) : nullptr;
    return refresh ? refresh->settingsRefreshFields(configuration) : QSet<QString>{};
}

SettingsDependencies Host::settingsDependencies(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    return module ? dependenciesFor(*module->instance, configuration) : SettingsDependencies{};
}

Configuration Host::applySettings(const QString& id, Configuration configuration, const QJsonObject& values) {
    const auto module = data_->modules.value(id);
    return module ? module->instance->applySettings(std::move(configuration), values) : Configuration{};
}
std::optional<EmulatorSelection> Host::emulatorSelection(const QString& id, const Configuration& configuration,
    std::stop_token token) const {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleEmulatorManager*>(module->instance) : nullptr;
    if (!manager || token.stop_requested()) return {};
    EmulatorSelection selection;
    selection.available = manager->emulatorInstallations(configuration, token);
    if (token.stop_requested() || selection.available.isEmpty()) return selection;
    QSet<QString> ids;
    for (const auto& item : selection.available) {
        const auto& emulator = item.emulator;
        if (emulator.id.trimmed().isEmpty() || emulator.displayName.trimmed().isEmpty() ||
            emulator.descriptionResourceKey.trimmed().isEmpty() || ids.contains(emulator.id))
            return EmulatorSelection{};
        ids.insert(emulator.id);
    }
    selection.selectedId = manager->emulatorInstallation(configuration, token).emulator.id;
    if (!ids.contains(selection.selectedId) || token.stop_requested()) return EmulatorSelection{};
    const auto saved = module->instance->loadConfigurations(token);
    selection.locked = std::ranges::find(saved, configuration.machineId, &Configuration::machineId) != saved.end();
    return selection;
}

Configuration Host::useEmulator(const QString& id, Configuration configuration, const QString& emulatorId,
    std::stop_token token) {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleEmulatorManager*>(module->instance) : nullptr;
    if (!manager || configuration.id.isNull() || emulatorId.isEmpty() || token.stop_requested()) return {};
    const auto selection = emulatorSelection(id, configuration, token);
    if (!selection || selection->selectedId.isEmpty() || token.stop_requested()) return {};
    if (selection->selectedId == emulatorId) return configuration;
    if (selection->locked || !std::ranges::any_of(selection->available, [&](const auto& item) {
        return item.emulator.id == emulatorId;
    })) return {};
    const auto configurationId = configuration.id;
    const auto machineId = configuration.machineId;
    auto updated = manager->useEmulator(std::move(configuration), emulatorId, token);
    if (token.stop_requested() || updated.id != configurationId || updated.machineId != machineId ||
        manager->emulatorInstallation(updated, token).emulator.id != emulatorId) return {};
    return updated;
}

QList<EmulatorRelease> Host::findEmulatorReleases(const QString& id, const Configuration& configuration,
    std::stop_token token) const {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleEmulatorManager*>(module->instance) : nullptr;
    if (!manager || token.stop_requested() || !manager->emulatorInstallation(configuration, token).canSearchReleases)
        return {};
    return manager->findEmulatorReleases(configuration, token);
}

QString Host::installEmulator(const QString& id, const Configuration& configuration, const EmulatorRelease& release,
    const EmulatorInstallProgress& progress, std::stop_token token) {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleEmulatorManager*>(module->instance) : nullptr;
    if (!manager || release.id.isEmpty() || token.stop_requested() ||
        !manager->emulatorInstallation(configuration, token).canSearchReleases) return {};
    return manager->installEmulator(configuration, release, progress, token);
}

QString Host::firmwareDirectory(const QString& id, const QString& machineId) const {
    const auto module = data_->modules.value(id);
    const auto* manager = module ? dynamic_cast<const IModuleFirmwareManager*>(module->instance) : nullptr;
    return manager ? manager->firmwareDirectory(machineId) : QString{};
}

QList<FirmwareCandidate> Host::scanFirmware(const QString& id, const Configuration& configuration,
    std::stop_token token) const {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleFirmwareManager*>(module->instance) : nullptr;
    return manager && !token.stop_requested() ? manager->scanFirmware(configuration.machineId, configuration, token)
        : QList<FirmwareCandidate>{};
}

Configuration Host::useFirmware(const QString& id, Configuration configuration, const FirmwareCandidate& candidate) {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleFirmwareManager*>(module->instance) : nullptr;
    return manager ? manager->useFirmware(std::move(configuration), candidate) : configuration;
}

std::optional<StorageSettings> Host::storageSettings(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* manager = module ? dynamic_cast<const IModuleStorageSettingsManager*>(module->instance) : nullptr;
    return manager ? std::optional{manager->describeStorageSettings(configuration)} : std::nullopt;
}

Configuration Host::applyStorageSettings(const QString& id, Configuration configuration, const StorageSettings& settings) {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleStorageSettingsManager*>(module->instance) : nullptr;
    if (!manager || configuration.id.isNull()) return {};
    auto draft = settings;
    draft.availableDevices = manager->describeStorageSettings(configuration).availableDevices;
    return applyStorageDraft(*manager, std::move(configuration), std::move(draft));
}

Configuration Host::applyStorageSettings(const QString& id, Configuration configuration, const StorageSettings& settings,
    GraphicsApi api) {
    const auto module = data_->modules.value(id);
    auto* manager = module ? dynamic_cast<IModuleStorageSettingsManager*>(module->instance) : nullptr;
    if (!manager || configuration.id.isNull()) return {};
    auto draft = settings;
    const auto configured = runtimeConfiguration(*module->instance, configuration, api);
    draft.availableDevices = manager->describeStorageSettings(configured).availableDevices;
    return applyStorageDraft(*manager, std::move(configuration), std::move(draft));
}

std::optional<InputSettings> Host::inputSettings(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* settings = module ? dynamic_cast<const IModuleInputSettingsManager*>(&*module->instance) : nullptr;
    if (!settings || configuration.id.isNull()) return {};
    return settings->describeInputSettings(configuration);
}

Configuration Host::applyInputSettings(const QString& id, Configuration configuration, const InputSettings& input) {
    const auto module = data_->modules.value(id);
    auto* settings = module ? dynamic_cast<IModuleInputSettingsManager*>(&*module->instance) : nullptr;
    if (!settings || configuration.id.isNull()) return {};
    const auto configurationId = configuration.id;
    const auto machineId = configuration.machineId;
    auto updated = settings->applyInputSettings(std::move(configuration), input);
    return updated.id == configurationId && updated.machineId == machineId ? std::move(updated) : Configuration{};
}

bool Host::saveInputSettings(const QString& id, const Configuration& configuration, std::stop_token token) {
    if (configuration.id.isNull() || token.stop_requested()) return false;
    const auto module = data_->modules.value(id);
    auto* settings = module ? dynamic_cast<IModuleInputSettingsManager*>(&*module->instance) : nullptr;
    if (!settings) return false;
    settings->saveInputSettings(configuration, token);
    return !token.stop_requested();
}

QList<ControllerPortFeedback> Host::controllerFeedbackPorts(const QString& id, const Configuration& configuration) const {
    const auto module = data_->modules.value(id);
    const auto* feedback = module ? dynamic_cast<const IModuleControllerFeedbackSettings*>(module->instance) : nullptr;
    return feedback ? feedback->controllerFeedbackPorts(configuration) : QList<ControllerPortFeedback>{};
}
Configuration Host::applySettingsWithFeedback(const QString& id, Configuration configuration, const QJsonObject& values,
    const QList<ControllerPortFeedback>& ports) {
    if (ports.isEmpty()) return applySettings(id, std::move(configuration), values);
    const auto module = data_->modules.value(id);
    auto* feedback = module ? dynamic_cast<IModuleControllerFeedbackSettings*>(module->instance) : nullptr;
    if (!feedback) return {};
    const auto configurationId = configuration.id;
    const auto machineId = configuration.machineId;
    if (configurationId.isNull()) return {};
    if (!values.isEmpty()) configuration = module->instance->applySettings(std::move(configuration), values);
    if (configuration.id != configurationId || configuration.machineId != machineId) return {};
    auto updated = feedback->applyControllerFeedbackSettings(std::move(configuration), ports);
    return updated.id == configurationId && updated.machineId == machineId ? std::move(updated) : Configuration{};
}
void Host::saveConfiguration(const QString& id, const Configuration& configuration, std::stop_token token) {
    if (token.stop_requested()) return;
    const auto module = data_->modules.value(id);
    if (!module) return;
    auto& instance = *module->instance;
    if (dynamic_cast<IModuleEmulatorManager*>(&instance) &&
        !preservesEmulator(instance, configuration, instance.loadConfigurations(token), token)) {
        if (token.stop_requested()) return;
        throw std::invalid_argument(EmulatorRejected);
    }
    instance.saveConfiguration(configuration, token);
    if (!token.stop_requested() && dynamic_cast<IEmulationNetworkManager*>(&instance)) configurations(id, token);
}
void Host::deleteConfiguration(const QString& id, const QUuid& configuration, std::stop_token token) {
    const auto module = data_->modules.value(id);
    if (!module || token.stop_requested()) return;
    module->instance->deleteConfiguration(configuration, token);
    if (!token.stop_requested() && dynamic_cast<IEmulationNetworkManager*>(module->instance)) configurations(id, token);
}
std::shared_ptr<Session> Host::open(const QString& id, const Configuration& configuration, std::stop_token token) {
    return openWithGraphicsApi(id, configuration, GraphicsApi::Automatic, token);
}

std::shared_ptr<Session> Host::openWithGraphicsApi(const QString& id, const Configuration& configuration,
    GraphicsApi api, std::stop_token token) try {
    if (configuration.id.isNull() || token.stop_requested()) return {};
    for (const auto& session : data_->sessions)
        if (session->moduleId() == id && session->configuration().id == configuration.id) return session;
    const auto module = data_->modules.value(id);
    if (!module) {
        throw EmulationMessageException({MessageCategory::Emulator, MessageCode::EmulatorNotInstalled,
            MessageSeverity::Error, MessageTarget::Dialog, {id, configuration.machineId, {}, {}}, {}});
    }
    auto quickState = QDir(data_->stateDirectory).filePath(QString::fromLatin1(QuickStateFile)
        .arg(id, configuration.id.toString(QUuid::Id128)));
    const QDir moduleRoot(QDir(data_->storageDirectory).filePath(id));
    RuntimeServices services{*data_->media, moduleRoot.filePath(QLatin1String(SessionDirectory)),
        moduleRoot.filePath(QLatin1String(RuntimeStateDirectory)), moduleRoot.filePath(QLatin1String(ConvertedDirectory)),
        QCoreApplication::applicationFilePath()};
    auto state = std::make_unique<Session::Data>(Session::Data{module, data_->media, std::move(services), configuration,
        std::move(quickState), {}, {}});
    state->graphicsApi = api;
    const auto configured = runtimeConfiguration(*module->instance, configuration, api);
    const auto* storage = dynamic_cast<const IModuleStorageSettingsManager*>(module->instance);
    auto* mediaFactory = dynamic_cast<IModuleMountedMedia*>(module->instance);
    const auto desired = storage ? configuredMounts(*storage, configured) : QList<MountedMedia>{};
    state->machine = storage && mediaFactory
        ? mediaFactory->createRuntimeWithMedia(configured, state->services, desired, token)
        : module->instance->createRuntime(configured, state->services, token);
    if (!state->machine) return {};
    applyControllerFeedback(*state->machine, *module->instance, configuration);
    state->mounted = storage ? matchDevices(state->machine->mediaSlots(), desired)
        : state->machine->mountedMedia();
    auto session = std::shared_ptr<Session>(new Session(std::move(state)));
    data_->sessions.push_back(session);
    return session;
} catch (...) {
    std::rethrow_exception(translateError(std::current_exception(), MessageCategory::Machine,
        MessageCode::MachineStartFailed, {id, configuration.machineId, {}, {}}));
}
const QList<std::shared_ptr<Session>>& Host::sessions() const noexcept { return data_->sessions; }
void Host::close(const std::shared_ptr<Session>& session) {
    const auto closing = session;
    if (data_->sessions.removeAll(closing) == 0) return;
    closing->stopForClose();
}
}
