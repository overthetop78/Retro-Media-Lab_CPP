#include <ErrorKeys.h>
#include "WorkspaceState.h"
#include "Media/CaptureSession.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
void WorkspaceState::ensureMediaOperations(Translations::TranslationService& translations) {
    if(!greaseweazleTools)greaseweazleTools=std::make_shared<GreaseweazleTools>(logsDirectory,[this]{return logging();},[&translations](const QString& key){return QString::fromStdString(translations.text(key.toStdString()));});
    const auto initialize=[this](auto& session,Infrastructure::CaptureMode mode) {
        if(session)return;
        session=std::make_shared<Infrastructure::CaptureSession>(mode,greaseweazleTools);
        session->draft.directory=defaultImageDirectory();
        session->output=[this](const QString& command,const QString& output){console.setContent(command,output);};
    };
    initialize(reading,Infrastructure::CaptureMode::Read);initialize(writing,Infrastructure::CaptureMode::Write);
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
