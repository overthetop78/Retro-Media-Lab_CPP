#pragma once

#include "AppApi.h"
#include "State/EmulationFolders.h"
#include <RetroMediaLab/Emulation/ControllerFeedback.h>
#include <RetroMediaLab/Emulation/Firmware.h>
#include <RetroMediaLab/Emulation/InputSettings.h>
#include <RetroMediaLab/VideoPresentation/Presentation.h>

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

namespace RetroMediaLab {
struct WorkspaceState;

RETROMEDIALAB_APP_API EmulationFolders resolveEmulationFolders(const EmulationFolders&);
RETROMEDIALAB_APP_API void ensureEmulationHost(WorkspaceState&);
RETROMEDIALAB_APP_API VideoPresentation::ProfileStore emulationVideoProfiles();
inline constexpr auto MachineVideoHardwareKey = "Emulation.Video.Hardware";
inline constexpr auto EmulationModuleIssueKey = "Emulation.Native.ModuleIssue";

RETROMEDIALAB_APP_API QList<Emulation::SettingsBlock> machineSettingsEditorBlocks(
    QList<Emulation::SettingsBlock>, const Emulation::SettingsDependencies&);

RETROMEDIALAB_APP_API QString saveEmulationScreenshot(const QImage&, const QString& directory);

struct MachineConfigurationDraft {
    Emulation::Configuration configuration;
    std::optional<Emulation::InputSettings> input;
};

class EmulationConfigurationDrafts final {
public:
    RETROMEDIALAB_APP_API std::optional<MachineConfigurationDraft> find(const QString& module, const QString& machine) const;
    RETROMEDIALAB_APP_API void set(const QString& module, const MachineConfigurationDraft&);
    RETROMEDIALAB_APP_API void remove(const QString& module, const QString& machine);

private:
    QHash<std::pair<QString, QString>, MachineConfigurationDraft> drafts_;
};

enum class MachineSettingsResult { Rejected, Drafted, Saved, EmulatorChanged };
enum class MachineSettingsSave { Persist, KeepDraft };
RETROMEDIALAB_APP_API MachineSettingsResult saveMachineSettings(
    Emulation::Host&, Emulation::Session*, const QString& module, Emulation::Configuration&,
    QJsonObject changes, const QList<Emulation::ControllerPortFeedback>&,
    const QList<Emulation::FirmwareCandidate>&, const Emulation::StorageSettings*,
    const Emulation::InputSettings*, const QString& requestedEmulator,
    MachineSettingsSave, Emulation::GraphicsApi);
}
