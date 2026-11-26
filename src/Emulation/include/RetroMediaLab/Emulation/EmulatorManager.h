#pragma once

#include "Sdk.h"
#include <functional>

namespace RetroMediaLab::Emulation {
struct EmulatorDefinition {
    QString id;
    QString displayName;
    QString descriptionResourceKey;
    QSet<QString> machineIds;
};

struct EmulatorInstallation {
    EmulatorDefinition emulator;
    std::optional<QString> installedVersion;
    bool canSearchReleases = true;
};

struct EmulatorRelease {
    QString id;
    QString displayName;
    QString version;
    bool required = false;
};

struct EmulatorSelection {
    QList<EmulatorInstallation> available;
    QString selectedId;
    bool locked = false;
};

using EmulatorInstallProgress = std::function<void(double)>;

class IModuleEmulatorManager {
public:
    virtual ~IModuleEmulatorManager() = default;
    virtual EmulatorInstallation emulatorInstallation(const Configuration&, std::stop_token) = 0;
    virtual QList<EmulatorInstallation> emulatorInstallations(const Configuration& configuration, std::stop_token token) {
        return {emulatorInstallation(configuration, token)};
    }
    virtual Configuration useEmulator(Configuration configuration, const QString& emulatorId, std::stop_token token) {
        return emulatorInstallation(configuration, token).emulator.id == emulatorId
            ? std::move(configuration) : Configuration{};
    }
    virtual QList<EmulatorRelease> findEmulatorReleases(const Configuration&, std::stop_token) = 0;
    virtual QString installEmulator(const Configuration&, const EmulatorRelease&, const EmulatorInstallProgress&,
        std::stop_token) = 0;
};
}
