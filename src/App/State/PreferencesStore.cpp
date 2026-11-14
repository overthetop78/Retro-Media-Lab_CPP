#include "PreferencesStore.h"
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <stdexcept>
namespace RetroMediaLab {
namespace {
void validate(const UserPreferences& value) {
    if (!Translations::isSupportedLanguage(value.language.toStdString()))
        throw std::runtime_error("Unsupported preference language");
    if (value.variant != ThemeVariant::Light && value.variant != ThemeVariant::Dark)
        throw std::runtime_error("Invalid appearance preference");
    if (value.defaultImageDirectory.contains(QChar::Null))
        throw std::runtime_error("Invalid default image directory");
}
}
UserPreferences PreferencesStore::load(const UserPreferences& defaults) const {
    validate(defaults);
    QFile file(path_);
    if (!file.exists()) return defaults;
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot read preferences");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    const auto data = document.object();
    if (error.error != QJsonParseError::NoError || !document.isObject()
        || data.value("version").toInt() != 1 || !data.value("language").isString()
        || (data.value("appearance") != "light" && data.value("appearance") != "dark"))
        throw std::runtime_error("Invalid preferences file");
    UserPreferences result{data.value("language").toString(),
        data.value("appearance") == "dark" ? ThemeVariant::Dark : ThemeVariant::Light};
    const auto directory = data.value("defaultImageDirectory");
    if (!directory.isUndefined() && !directory.isString())
        throw std::runtime_error("Invalid default image directory preference");
    result.defaultImageDirectory = directory.isUndefined() ? defaults.defaultImageDirectory : directory.toString();
    validate(result);
    return result;
}
void PreferencesStore::save(const UserPreferences& value) const {
    validate(value);
    const auto directory = QFileInfo(path_).absolutePath();
    if (!QDir().mkpath(directory)) throw std::runtime_error("Cannot create preferences directory");
    QSaveFile file(path_);
    const auto bytes = QJsonDocument(QJsonObject{
        {"version", 1}, {"language", value.language},
        {"appearance", value.variant == ThemeVariant::Dark ? "dark" : "light"},
        {"defaultImageDirectory", value.defaultImageDirectory}
    }).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Cannot save preferences");
}
}
