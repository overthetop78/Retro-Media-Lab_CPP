#pragma once
#include "AppApi.h"
#include "Themes/ThemeService.h"
#include <QString>
#include <utility>
namespace RetroMediaLab {
struct UserPreferences {
    QString language = "en-US";
    ThemeVariant variant = ThemeVariant::Light;
    QString defaultImageDirectory;
};
class PreferencesStore final {
public:
    explicit PreferencesStore(QString path) : path_(std::move(path)) {}
    RETROMEDIALAB_APP_API UserPreferences load(const UserPreferences& defaults) const;
    RETROMEDIALAB_APP_API void save(const UserPreferences&) const;
private:
    QString path_;
};
}
