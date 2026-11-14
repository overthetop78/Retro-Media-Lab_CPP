#pragma once
#include <Constants/AppConstants.h>
#include "AppApi.h"
#include "Logging/LogSettings.h"
#include "Conversion/ConversionSettings.h"
#include "Hardware/HardwareSettings.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <mutex>
#include <QByteArray>
#include <QStringList>
#include <QMap>
#include "Shell/ConsoleState.h"
#include <functional>
#include <memory>
#include <vector>
namespace RetroMediaLab {
// Session-owned state survives destruction of theme windows. Business models will live outside views.
struct WorkspaceState {
    QStringList openPageIds{AppConstants::Read, AppConstants::Write, AppConstants::Convert, AppConstants::Visualize, AppConstants::Explorer, AppConstants::Tools, AppConstants::Emulation};
    QString activePageId = AppConstants::Read;
    ConsoleState console;
    bool preferencesOpen = false;
    QString preferencesSection = AppConstants::General;
    QByteArray preferencesGeometry;
    QByteArray mainWindowGeometry;
    bool mainWindowMaximized = false;
    bool mainWindowOpen = true;
    QString logsDirectory;
    QString hardwareCategory = HardwareConstants::Floppy;
    QString selectedHardwareDeviceId;
    QMap<QString, Infrastructure::OpticalCapabilities> opticalCapabilities;
    QMap<QString,QString> hardwareProbeErrors;
    QMap<QString, Infrastructure::DiskAccessReport> diskInterfaceReports;
    QString diskControllerSelection;
    QVector<HardwareDevice> floppyReaderEdits; // Session drafts, committed only by each reader save action.
    RETROMEDIALAB_APP_API bool hasGeometry() const noexcept;
    using DirectorySubscription = std::shared_ptr<const std::function<void()>>;
    const QString& defaultImageDirectory() const noexcept { return defaultImageDirectory_; }
    RETROMEDIALAB_APP_API void setDefaultImageDirectory(QString);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToDefaultImageDirectory(std::function<void()>);
    using LoggingSubscription = DirectorySubscription;
    RETROMEDIALAB_APP_API LoggingSettings logging() const;
    RETROMEDIALAB_APP_API void setLogging(LoggingSettings);
    RETROMEDIALAB_APP_API LoggingSubscription subscribeToLogging(std::function<void()>);
    const ConversionSettings& conversion() const noexcept { return conversion_; }
    RETROMEDIALAB_APP_API void setConversion(ConversionSettings);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToConversion(std::function<void()>);
    const HardwareSettings& hardware() const noexcept { return hardware_; }
    RETROMEDIALAB_APP_API void setHardware(HardwareSettings);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToHardware(std::function<void()>);
private:
    HardwareSettings hardware_;
    std::vector<std::weak_ptr<const std::function<void()>>> hardwareObservers_;
    ConversionSettings conversion_;
    std::vector<std::weak_ptr<const std::function<void()>>> conversionObservers_;
    QString defaultImageDirectory_;
    std::vector<std::weak_ptr<const std::function<void()>>> directoryObservers_;
    mutable std::mutex loggingMutex_;
    LoggingSettings logging_;
    std::vector<std::weak_ptr<const std::function<void()>>> loggingObservers_;
};
}
