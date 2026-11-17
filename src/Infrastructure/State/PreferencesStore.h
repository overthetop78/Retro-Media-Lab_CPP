#pragma once
#include <Constants/AppConstants.h>
#include <Constants/DisplayConstants.h>
#include "InfrastructureApi.h"
#include "Logging/LogSettings.h"
#include "Engines/EngineSettings.h"
#include "Conversion/ConversionSettings.h"
#include "Hardware/HardwareSettings.h"
#include "Profiles/OperationProfiles.h"
#include <QJsonObject>
#include <QString>
#include <utility>
namespace RetroMediaLab {
struct UserPreferences {
    QString language = AppConstants::EnUS;
    QString defaultImageDirectory;
    QJsonObject themeSettings;
    LoggingSettings logging;
    ConversionSettings conversion;
    HardwareSettings hardware;
    EngineSettings engines;
    int interfaceScale = DisplayConstants::DefaultPercent;
    QJsonArray profiles;
};
class PreferencesStore final {
public:
    explicit PreferencesStore(QString path) : path_(std::move(path)) {}
    RETROMEDIALAB_INFRASTRUCTURE_API static QString defaultPath();
    RETROMEDIALAB_INFRASTRUCTURE_API UserPreferences load(const UserPreferences& defaults) const;
    RETROMEDIALAB_INFRASTRUCTURE_API UserPreferences loadOrCreate(const UserPreferences& defaults) const;
    RETROMEDIALAB_INFRASTRUCTURE_API void save(const UserPreferences&) const;
private:
    QString path_;
};
}
