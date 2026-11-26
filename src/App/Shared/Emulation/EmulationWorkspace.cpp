#include "Shared/Emulation/EmulationWorkspace.h"
#include "State/WorkspaceState.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QSaveFile>
#include <QStandardPaths>

namespace RetroMediaLab {
namespace {
constexpr auto ModulesDirectory = "Modules";
constexpr auto ConfigurationsDirectory = "Emulation";
constexpr auto ProfilesDirectory = "VideoPresentation";

constexpr auto DefaultEmulationStorage = "Retro Media Lab/Emulation";
constexpr auto CapturesDirectory = "Captures";
constexpr auto StatesDirectory = "States";
constexpr auto CaptureFormat = "PNG";
constexpr auto CaptureStemFormat = "capture-%1";
constexpr auto CaptureTimestampFormat = "yyyyMMdd-HHmmss";
constexpr auto CaptureExtension = ".png";
constexpr auto CaptureDuplicateFormat = "%1-%2%3";
constexpr int FirstDuplicateCaptureSuffix = 2;

QDir settingsDirectory() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
}
}

EmulationFolders resolveEmulationFolders(const EmulationFolders& folders) {
    const auto resolve = [](const QString& configured, QString fallback) {
        const auto value = configured.trimmed();
        return value.isEmpty() ? std::move(fallback) : QDir(value).absolutePath();
    };
    const auto storage = resolve(folders.storage, QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
        .filePath(QLatin1String(DefaultEmulationStorage)));
    const QDir root(storage);
    return {storage, resolve(folders.captures, root.filePath(QLatin1String(CapturesDirectory))),
        resolve(folders.states, root.filePath(QLatin1String(StatesDirectory)))};
}

void ensureEmulationHost(WorkspaceState& state) {
    if (!state.emulationDrafts) state.emulationDrafts = std::make_shared<EmulationConfigurationDrafts>();
    if (state.emulation) return;
    auto host = std::make_shared<Emulation::Host>(
        QDir(QCoreApplication::applicationDirPath()).filePath(QLatin1String(ModulesDirectory)),
        settingsDirectory().filePath(QLatin1String(ConfigurationsDirectory)));
    const auto folders = resolveEmulationFolders(state.emulationFolders());
    host->setStorageDirectories(folders.storage, folders.states);
    host->discover();
    state.emulation = std::move(host);
}

VideoPresentation::ProfileStore emulationVideoProfiles() {
    const auto root = settingsDirectory();
    const QDir emulation(root.filePath(QLatin1String(ConfigurationsDirectory)));
    return VideoPresentation::ProfileStore(emulation.filePath(QLatin1String(ProfilesDirectory)), root.filePath(QLatin1String(ProfilesDirectory)));
}

QString saveEmulationScreenshot(const QImage& image, const QString& directory) {
    const QDir destination(directory.trimmed().isEmpty() ? resolveEmulationFolders({}).captures : directory.trimmed());
    if (image.isNull() || !QDir().mkpath(destination.absolutePath())) return {};
    const auto stem = QString::fromLatin1(CaptureStemFormat).arg(
        QDateTime::currentDateTime().toString(QLatin1String(CaptureTimestampFormat)));
    auto name = stem + QLatin1String(CaptureExtension);
    for (int suffix = FirstDuplicateCaptureSuffix; destination.exists(name); ++suffix)
        name = QString::fromLatin1(CaptureDuplicateFormat).arg(stem, QString::number(suffix), QLatin1String(CaptureExtension));
    QSaveFile file(destination.filePath(name));
    const bool saved = file.open(QIODevice::WriteOnly) && image.save(&file, CaptureFormat) && file.commit();
    return saved ? name : QString{};
}

std::optional<MachineConfigurationDraft> EmulationConfigurationDrafts::find(const QString& module, const QString& machine) const {
    const auto draft = drafts_.constFind({module, machine});
    return draft == drafts_.cend() ? std::nullopt : std::optional(*draft);
}

void EmulationConfigurationDrafts::set(const QString& module, const MachineConfigurationDraft& draft) {
    drafts_.insert({module, draft.configuration.machineId}, draft);
}

void EmulationConfigurationDrafts::remove(const QString& module, const QString& machine) {
    drafts_.remove({module, machine});
}

MachineSettingsResult saveMachineSettings(Emulation::Host& host, Emulation::Session* session,
    const QString& module, Emulation::Configuration& configuration, QJsonObject changes,
    const QList<Emulation::ControllerPortFeedback>& feedback, const QList<Emulation::FirmwareCandidate>& firmware,
    const Emulation::StorageSettings* storage, const Emulation::InputSettings* input, const QString& requestedEmulator,
    MachineSettingsSave save) {
    for (const auto& candidate : firmware) changes.remove(candidate.destinationFieldId);
    auto draft = input ? std::optional(*input) : std::nullopt;
    if (draft) {
        for (auto& port : draft->controllerPorts)
            if (auto outputs = Emulation::controllerModelFeedback(port)) port.outputBindings = std::move(outputs->outputs);
        input = &*draft;
    }
    if (session) {
        if (save == MachineSettingsSave::KeepDraft) return MachineSettingsResult::Rejected;
        const bool updated = input ? session->updateSettingsWithInput(changes, feedback, firmware, storage, *input)
            : storage ? session->updateSettingsWithStorage(changes, feedback, firmware, *storage)
            : session->updateSettingsWithFirmware(changes, feedback, firmware);
        if (!updated) return MachineSettingsResult::Rejected;
        configuration = session->configuration();
        return MachineSettingsResult::Saved;
    }

    auto updated = host.applySettingsWithFeedback(module, configuration, changes, feedback);
    for (const auto& candidate : firmware)
        updated = host.useFirmware(module, std::move(updated), candidate);
    if (updated.id.isNull()) return MachineSettingsResult::Rejected;
    if (storage) {
        updated = host.applyStorageSettings(module, std::move(updated), *storage);
        if (updated.id.isNull()) return MachineSettingsResult::Rejected;
    }
    if (!requestedEmulator.isEmpty()) {
        updated = host.useEmulator(module, std::move(updated), requestedEmulator);
        if (updated.id.isNull()) return MachineSettingsResult::Rejected;
        configuration = std::move(updated);
        return MachineSettingsResult::EmulatorChanged;
    }
    if (input) {
        updated = host.applyInputSettings(module, std::move(updated), *input);
        if (updated.id.isNull()) return MachineSettingsResult::Rejected;
    }
    if (save == MachineSettingsSave::KeepDraft) {
        configuration = std::move(updated);
        return MachineSettingsResult::Drafted;
    }
    host.saveConfiguration(module, updated);
    if (input && !host.saveInputSettings(module, updated)) return MachineSettingsResult::Rejected;
    configuration = std::move(updated);
    return MachineSettingsResult::Saved;
}
}
