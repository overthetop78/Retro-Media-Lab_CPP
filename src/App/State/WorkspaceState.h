#pragma once
#include <Constants/AppConstants.h>
#include <Constants/DisplayConstants.h>
#include "AppApi.h"
#include "Logging/LogSettings.h"
#include "Conversion/ConversionSettings.h"
#include "Hardware/HardwareSettings.h"
#include "Engines/EngineSettings.h"
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
namespace Translations {class TranslationService;}
namespace Infrastructure { class ExplorerSession; class VisualizationSession; class CaptureSession; class ConversionSession; }
class GreaseweazleTools;
// Session-owned state survives destruction of theme windows. Business models will live outside views.
struct WorkspaceState {
    std::shared_ptr<Infrastructure::ExplorerSession> explorer;
    std::shared_ptr<Infrastructure::VisualizationSession> visualization;
    QStringList openPageIds{AppConstants::Read, AppConstants::Write, AppConstants::Convert, AppConstants::Visualize, AppConstants::Explorer, AppConstants::Tools, AppConstants::Emulation};
    QString activePageId = AppConstants::Read;
    ConsoleState console;
    bool preferencesOpen = false;
    std::shared_ptr<GreaseweazleTools> greaseweazleTools;
    std::shared_ptr<Infrastructure::CaptureSession> reading,writing;
    std::shared_ptr<Infrastructure::ConversionSession> converting;
    RETROMEDIALAB_APP_API void ensureMediaOperations(Translations::TranslationService&);
    bool greaseweazleOpen = false;
    int greaseweazleTab = 0;
    QString greaseweazleDeviceId;
    QByteArray greaseweazleGeometry;
    QString preferencesSection = AppConstants::General;
    QByteArray preferencesGeometry;
    QByteArray mainWindowGeometry;
    bool mainWindowMaximized = false;
    bool mainWindowOpen = true;
    QString logsDirectory;
    QString engineCategory = EngineConstants::Greaseweazle;
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
    int interfaceScale() const noexcept { return interfaceScale_; }
    RETROMEDIALAB_APP_API void setInterfaceScale(int);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToInterfaceScale(std::function<void()>);
    const EngineSettings& engines() const noexcept { return engines_; }
    RETROMEDIALAB_APP_API void setEngines(EngineSettings);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToEngines(std::function<void()>);
    const HardwareSettings& hardware() const noexcept { return hardware_; }
    RETROMEDIALAB_APP_API void setHardware(HardwareSettings);
    RETROMEDIALAB_APP_API DirectorySubscription subscribeToHardware(std::function<void()>);
private:
    int interfaceScale_ = DisplayConstants::DefaultPercent;
    std::vector<std::weak_ptr<const std::function<void()>>> scaleObservers_;
    EngineSettings engines_;
    std::vector<std::weak_ptr<const std::function<void()>>> engineObservers_;
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
