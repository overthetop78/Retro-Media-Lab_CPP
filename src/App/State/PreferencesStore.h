#pragma once
#include <Constants/AppConstants.h>
#include "AppApi.h"
#include <QJsonObject>
#include <QString>
#include <utility>
namespace RetroMediaLab {
struct UserPreferences {
    QString language = AppConstants::EnUS;
    QString defaultImageDirectory;
    QJsonObject themeSettings;
};
class PreferencesStore final {
public:
    explicit PreferencesStore(QString path) : path_(std::move(path)) {}
    RETROMEDIALAB_APP_API UserPreferences load(const UserPreferences& defaults) const;
    RETROMEDIALAB_APP_API UserPreferences loadOrCreate(const UserPreferences& defaults) const;
    RETROMEDIALAB_APP_API void save(const UserPreferences&) const;
private:
    QString path_;
};
}
