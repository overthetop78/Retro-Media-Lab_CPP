#pragma once

#include "AudioOutputController.h"
#include "EmulatorManager.h"

#include <QUrl>
#include <concepts>
#include <type_traits>
#include <vector>

class QNetworkAccessManager;

namespace RetroMediaLab::Emulation {
struct EmulatorCatalogEntry {
    QString emulatorKey;
    QString id;
    QString libraryName;
    QString dynamicLibraryName;
    QString archiveName;
    QUrl archiveUri;
    QUrl sourceUri;
    QString inspectedRevision;
    QSet<QString> machineIds;
    std::optional<QString> descriptionResourceKey;

    EmulatorDefinition toDefinition() const {
        constexpr auto DescriptionKey = "Emulation.Emulator.%1.Description";
        return {id, libraryName, descriptionResourceKey.value_or(QString::fromLatin1(DescriptionKey).arg(id)),
            machineIds};
    }
};

// The configuration identifies exactly one of the module's models.
inline std::optional<QString> machineDisplayResourceKey(const QList<MachineDefinition>& models,
    const QString& machineId) {
    const MachineDefinition* match = nullptr;
    for (const auto& model : models) {
        if (model.id != machineId) continue;
        if (match) return std::nullopt;
        match = &model;
    }
    if (!match) return std::nullopt;
    return match->displayResourceKey;
}

struct EmulatorManagementContext {
    QNetworkAccessManager& network;
    QString coreDirectory;
};

template<class ModuleConfiguration>
struct EmulatorCreationContext {
    QString sessionsDirectory;
    QString corePath;
    QString hostExecutablePath;
    std::function<std::unique_ptr<IAudioOutput>()> audioOutputFactory;
    std::function<QString(const ModuleConfiguration&)> saveDirectoryResolver;
    QString firmwareDirectory;
};

// A module owns its adapters. They borrow the management services and own the
// resources they create for each machine.
template<class ModuleConfiguration, std::derived_from<IEmulatedMachine> Machine>
class IEmulatorAdapter {
public:
    virtual ~IEmulatorAdapter() = default;

    virtual QString emulatorId() const = 0;
    virtual QString emulatorKey() const = 0;
    virtual EmulatorDefinition definition() const = 0;
    virtual std::optional<int> tryHandleHostCommand(const QStringList& arguments) = 0;
    virtual EmulatorInstallation installation(const EmulatorManagementContext&, std::stop_token) = 0;
    virtual QList<EmulatorRelease> findReleases(const EmulatorManagementContext&, std::stop_token) = 0;
    virtual QString install(const EmulatorManagementContext&, const EmulatorRelease&,
        const EmulatorInstallProgress&, std::stop_token) = 0;
    virtual std::optional<QString> findInstalledCorePath(const EmulatorManagementContext&, std::stop_token) = 0;
    virtual QList<MountedMedia> resolveConfiguredMedia(const ModuleConfiguration&) = 0;
    virtual std::unique_ptr<Machine> create(const ModuleConfiguration&,
        const EmulatorCreationContext<ModuleConfiguration>&) = 0;
};

// The module declares its concrete types in discovery order. Instantiation
// itself is limited to types that satisfy both of the requested contracts.
template<class Factory, class Adapter, class... Types>
    requires std::has_virtual_destructor_v<Factory>
std::vector<std::unique_ptr<Factory>> createEmulatorFactories() {
    std::vector<std::unique_ptr<Factory>> factories;
    const auto add = [&]<class Type> {
        if constexpr (std::derived_from<Type, Factory> && std::derived_from<Type, Adapter> &&
                      !std::is_abstract_v<Type>) {
            factories.push_back(std::make_unique<Type>());
        }
    };
    (add.template operator()<Types>(), ...);
    return factories;
}

template<class Adapter, class... Types>
    requires std::has_virtual_destructor_v<Adapter>
std::vector<std::unique_ptr<Adapter>> createEmulatorAdapters() {
    return createEmulatorFactories<Adapter, Adapter, Types...>();
}
}
