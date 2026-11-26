#include "Shared/Controls/RememberedFileDialogConstants.h"
#include "Media/ExplorerReadSession.h"
#include "Media/MigrationSession.h"
#include "Media/VisualizationSession.h"
#include <ErrorKeys.h>
#include "WorkspaceState.h"
#include "Shared/Emulation/EmulationWorkspace.h"
#include "Media/CaptureSession.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include "Media/ConversionSession.h"
#include "Shared/Localization/MediaText.h"
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
void WorkspaceState::ensureMediaOperations(Translations::TranslationService& translations) {
    if(!greaseweazleTools)greaseweazleTools=std::make_shared<GreaseweazleTools>(logsDirectory,[this]{return logging();},[&translations](const QString& key){return QString::fromStdString(translations.text(key.toStdString()));});
    const auto initialize=[this](auto& session,Infrastructure::CaptureMode mode) {
        if(session)return;
        session=std::make_shared<Infrastructure::CaptureSession>(mode,greaseweazleTools,profiles);
        session->draft.directory=mode==Infrastructure::CaptureMode::Read
            ? fileDialogDirectories().value(FileDialogScope::ReadDestination).toString(defaultImageDirectory())
            : defaultImageDirectory();
        session->output=[this](const QString& command,const QString& output){console.setContent(command,output);};
    };
    initialize(reading,Infrastructure::CaptureMode::Read);initialize(writing,Infrastructure::CaptureMode::Write);
    if(!migration)migration=std::make_shared<Infrastructure::MigrationSession>();
    if(!explorer)explorer=std::make_shared<Infrastructure::ExplorerSession>();
    if(!visualization)visualization=std::make_shared<Infrastructure::VisualizationSession>(explorer);
    if(!explorerReading){explorerReading=std::make_shared<Infrastructure::ExplorerReadSession>(explorer,greaseweazleTools,profiles);explorerReading->capture().output=[this](const QString& command,const QString& output){console.setContent(command,output);};}
    if(!converting){
        converting=std::make_shared<Infrastructure::ConversionSession>(greaseweazleTools,logsDirectory,[this]{return logging();},[&translations](const auto& message){return mediaText(translations,message);},profiles);
        converting->draft.addTags=conversion().addTags;
        converting->output=[this](const QString& command,const QString& output){console.setContent(command,output);};
    }
}
bool WorkspaceState::hasGeometry() const noexcept { return !mainWindowGeometry.isEmpty(); }
void WorkspaceState::setDefaultImageDirectory(QString directory) {
    if (directory.contains(QChar::Null)) throw std::invalid_argument(ErrorKeys::InvalidImageDirectory);
    if (directory == defaultImageDirectory_) return;
    defaultImageDirectory_ = std::move(directory);
    const auto observers = directoryObservers_;
    std::erase_if(directoryObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToDefaultImageDirectory(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    directoryObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setFileDialogDirectory(const QString& scope, const QString& directory) {
    if (scope.isEmpty() || directory.contains(QChar::Null))
        throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
    const auto value = fileDialogDirectories_.value(scope);
    if (value.isString() && value.toString() == directory) return;
    fileDialogDirectories_.insert(scope, directory);
    const auto observers = fileDialogDirectoryObservers_;
    std::erase_if(fileDialogDirectoryObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToFileDialogDirectories(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    fileDialogDirectoryObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setEmulationFolders(EmulationFolders folders) {
    if (folders.storage.contains(QChar::Null) || folders.captures.contains(QChar::Null) ||
        folders.states.contains(QChar::Null))
        throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
    folders = {folders.storage.trimmed(), folders.captures.trimmed(), folders.states.trimmed()};
    if (folders == emulationFolders_) return;
    const auto resolved = resolveEmulationFolders(folders);
    const auto observers = emulationFolderObservers_;
    auto previous = std::exchange(emulationFolders_, std::move(folders));
    std::erase_if(emulationFolderObservers_, [](const auto& observer) { return observer.expired(); });
    try {
        for (const auto& observer : observers)
            if (const auto live = observer.lock()) (*live)();
    } catch (...) {
        emulationFolders_ = std::move(previous);
        throw;
    }
    if (emulation) emulation->setStorageDirectories(resolved.storage, resolved.states);
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToEmulationFolders(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    emulationFolderObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setEmulationShortcuts(QJsonObject shortcuts) {
    if (shortcuts == emulationShortcuts_) return;
    const auto observers = emulationShortcutObservers_;
    auto previous = std::exchange(emulationShortcuts_, std::move(shortcuts));
    std::erase_if(emulationShortcutObservers_, [](const auto& observer) { return observer.expired(); });
    try {
        for (const auto& observer : observers)
            if (const auto live = observer.lock()) (*live)();
    } catch (...) {
        emulationShortcuts_ = std::move(previous);
        throw;
    }
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToEmulationShortcuts(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    emulationShortcutObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setShowFullscreenEmulationStatistics(bool visible) {
    if (visible == showFullscreenEmulationStatistics_) return;
    showFullscreenEmulationStatistics_ = visible;
    const auto observers = emulationStatisticsObservers_;
    std::erase_if(emulationStatisticsObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToEmulationStatistics(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    emulationStatisticsObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setFullscreenOsdTransparency(int percent) {
    percent = std::clamp(percent, DisplayConstants::MinimumOsdTransparency, DisplayConstants::MaximumOsdTransparency);
    if (percent == fullscreenOsdTransparency_) return;
    const auto observers = osdTransparencyObservers_;
    const int previous = std::exchange(fullscreenOsdTransparency_, percent);
    std::erase_if(osdTransparencyObservers_, [](const auto& observer) { return observer.expired(); });
    try {
        for (const auto& observer : observers)
            if (const auto live = observer.lock()) (*live)();
    } catch (...) {
        fullscreenOsdTransparency_ = previous;
        throw;
    }
}

WorkspaceState::DirectorySubscription WorkspaceState::subscribeToOsdTransparency(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    osdTransparencyObservers_.push_back(subscription);
    return subscription;
}

LoggingSettings WorkspaceState::logging() const {
    const std::lock_guard lock(loggingMutex_);return logging_;
}
void WorkspaceState::setLogging(LoggingSettings settings) {
    settings.validate();
    {const std::lock_guard lock(loggingMutex_);if(settings==logging_)return;logging_=std::move(settings);}
    const auto observers=loggingObservers_;
    std::erase_if(loggingObservers_,[](const auto& observer){return observer.expired();});
    for(const auto& observer:observers)if(const auto live=observer.lock())(*live)();
}
WorkspaceState::LoggingSubscription WorkspaceState::subscribeToLogging(std::function<void()> observer) {
    auto subscription=std::make_shared<const std::function<void()>>(std::move(observer));
    std::erase_if(loggingObservers_,[](const auto& item){return item.expired();});
    loggingObservers_.push_back(subscription);return subscription;
}
void WorkspaceState::setConversion(ConversionSettings settings) {
    if (settings == conversion_) return;
    conversion_ = std::move(settings);
    const auto observers = conversionObservers_;
    std::erase_if(conversionObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToConversion(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    conversionObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setHardware(HardwareSettings settings) {
    if (settings == hardware_) return;
    hardware_ = std::move(settings);
    const auto observers = hardwareObservers_;
    std::erase_if(hardwareObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToHardware(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    hardwareObservers_.push_back(subscription);
    return subscription;
}
void WorkspaceState::setEngines(EngineSettings settings){
    settings.validate();if(settings==engines_)return;engines_=std::move(settings);
    const auto observers=engineObservers_;std::erase_if(engineObservers_,[](const auto& item){return item.expired();});
    for(const auto& observer:observers)if(const auto live=observer.lock())(*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToEngines(std::function<void()> observer){
    std::erase_if(engineObservers_,[](const auto& item){return item.expired();});
    auto result=std::make_shared<const std::function<void()>>(std::move(observer));engineObservers_.push_back(result);return result;
}
void WorkspaceState::setInterfaceScale(int value) {
    if (!DisplayConstants::supported(value)) throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
    if (value == interfaceScale_) return;
    interfaceScale_ = value;
    const auto observers = scaleObservers_;
    std::erase_if(scaleObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToInterfaceScale(std::function<void()> observer) {
    auto result = std::make_shared<const std::function<void()>>(std::move(observer));
    std::erase_if(scaleObservers_, [](const auto& item) { return item.expired(); });
    scaleObservers_.push_back(result);
    return result;
}

}
