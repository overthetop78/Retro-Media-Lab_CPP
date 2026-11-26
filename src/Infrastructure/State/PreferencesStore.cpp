#include <ErrorKeys.h>
#include <Constants/AppConstants.h>
#include <Constants/FileDialogPreferences.h>
#include "PreferencesStore.h"
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <stdexcept>
namespace RetroMediaLab {
QString PreferencesStore::defaultPath(){return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+AppConstants::PreferencesJson;}
namespace {
constexpr auto EmulationShortcutsKey = "emulationShortcuts";
constexpr auto StorageDirectoryKey = "emulationStorageFolder";
constexpr auto CaptureDirectoryKey = "emulationCaptureFolder";
constexpr auto StateDirectoryKey = "emulationStateFolder";
constexpr auto FullscreenStatisticsKey = "showFullscreenEmulationStatistics";

void validate(const UserPreferences& value) {
    if (!DisplayConstants::supported(value.interfaceScale)) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    value.logging.validate();
    value.engines.validate();
    Infrastructure::OperationProfiles profiles;profiles.reset(value.profiles);
    if (!Translations::isSupportedLanguage(value.language.toStdString()))
        throw std::runtime_error(ErrorKeys::UnsupportedPreferenceLanguage);
    for (auto it = value.themeSettings.begin(); it != value.themeSettings.end(); ++it)
        if (it.key().isEmpty() || !it.value().isObject()) throw std::runtime_error(ErrorKeys::InvalidThemeSettings);
    if (value.fileDialogDirectories) {
        for (auto it = value.fileDialogDirectories->constBegin(); it != value.fileDialogDirectories->constEnd(); ++it) {
            if (it.key().isEmpty() || !it.value().isString() || it.value().toString().contains(QChar::Null))
                throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
        }
    }
    for (auto binding = value.emulationShortcuts.constBegin(); binding != value.emulationShortcuts.constEnd(); ++binding) {
        if (binding.key().isEmpty() || !binding.value().isString() || binding.value().toString().contains(QChar::Null))
            throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    }
    if (value.emulationFolders.storage.contains(QChar::Null) || value.emulationFolders.captures.contains(QChar::Null) ||
        value.emulationFolders.states.contains(QChar::Null))
        throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    if (value.defaultImageDirectory.contains(QChar::Null))
        throw std::runtime_error(ErrorKeys::InvalidDefaultImageDirectory);
}
}
UserPreferences PreferencesStore::load(const UserPreferences& defaults) const {
    validate(defaults);
    QFile file(path_);
    if (!file.exists()) return defaults;
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(ErrorKeys::CannotReadPreferences);
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    const auto data = document.object();
    const auto version = data.value(AppConstants::Version).toInt();
    if (error.error != QJsonParseError::NoError || !document.isObject()
        || (version != AppConstants::PreferencesStoreValues::LegacySchemaVersion && version != AppConstants::PreferencesStoreValues::CurrentSchemaVersion) || !data.value(AppConstants::Language).isString())
        throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    UserPreferences result = defaults;
    const auto scale = data.value(DisplayConstants::Preference);
    if (!scale.isUndefined() && (!scale.isDouble() || scale.toDouble() != scale.toInt())) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    result.interfaceScale = scale.isUndefined() ? defaults.interfaceScale : scale.toInt();
    result.language = data.value(AppConstants::Language).toString();
    result.engines = EngineSettings::fromJson(data.value(EngineConstants::Section));
    result.hardware = HardwareSettings::fromJson(data.value(HardwareConstants::Section));
    result.conversion = ConversionSettings::fromJson(data.value(ConversionConstants::Section));
    result.logging = LoggingSettings::fromJson(data.value(LogConstants::Logging));
    Infrastructure::OperationProfiles profiles;profiles.reset(data.value(Infrastructure::ProfileConstants::Section));result.profiles=profiles.toJson();
    if (version == AppConstants::PreferencesStoreValues::LegacySchemaVersion) {
        const auto appearance = data.value(AppConstants::Appearance);
        if (appearance != AppConstants::Light && appearance != AppConstants::Dark && appearance != AppConstants::System)
            throw std::runtime_error(ErrorKeys::InvalidLegacyAppearancePreference);
        result.themeSettings.insert(AppConstants::RetroMediaLab, QJsonObject{{AppConstants::Appearance, appearance}});
    } else {
        if (!data.value(AppConstants::ThemeSettings).isObject()) throw std::runtime_error(ErrorKeys::InvalidThemeSettings);
        result.themeSettings = data.value(AppConstants::ThemeSettings).toObject();
    }
    const auto directory = data.value(AppConstants::DefaultImageDirectory);
    if (!directory.isUndefined() && !directory.isString())
        throw std::runtime_error(ErrorKeys::InvalidDefaultImageDirectoryPreference);
    result.defaultImageDirectory = directory.isUndefined() ? defaults.defaultImageDirectory : directory.toString();
    const auto dialogDirectories = data.value(FileDialogPreferences::JsonKey);
    if (!dialogDirectories.isUndefined() && !dialogDirectories.isObject())
        throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    if (dialogDirectories.isObject()) result.fileDialogDirectories = dialogDirectories.toObject();
    else result.fileDialogDirectories.reset();
    const auto shortcuts = data.value(EmulationShortcutsKey);
    if (!shortcuts.isUndefined() && !shortcuts.isObject()) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    result.emulationShortcuts = shortcuts.toObject(defaults.emulationShortcuts);
    const auto folder = [&](const char* key, const QString& fallback) {
        const auto value = data.value(QLatin1String(key));
        if (!value.isUndefined() && !value.isString()) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
        return value.toString(fallback);
    };
    result.emulationFolders = {folder(StorageDirectoryKey, defaults.emulationFolders.storage),
        folder(CaptureDirectoryKey, defaults.emulationFolders.captures),
        folder(StateDirectoryKey, defaults.emulationFolders.states)};
    const auto statistics = data.value(FullscreenStatisticsKey);
    if (!statistics.isUndefined() && !statistics.isBool()) throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);
    result.showFullscreenEmulationStatistics = statistics.toBool(defaults.showFullscreenEmulationStatistics);
    validate(result);
    return result;
}
void PreferencesStore::save(const UserPreferences& value) const {
    validate(value);
    const auto directory = QFileInfo(path_).absolutePath();
    if (!QDir().mkpath(directory)) throw std::runtime_error(ErrorKeys::CannotCreatePreferencesDirectory);
    QSaveFile file(path_);
    const auto bytes = QJsonDocument(QJsonObject{
        {AppConstants::Version, AppConstants::PreferencesStoreValues::CurrentSchemaVersion}, {AppConstants::Language, value.language},
        {AppConstants::ThemeSettings, value.themeSettings},
        {LogConstants::Logging, value.logging.toJson()},
        {ConversionConstants::Section, value.conversion.toJson()},
        {HardwareConstants::Section, value.hardware.toJson()},
        {EngineConstants::Section, value.engines.toJson()},
        {Infrastructure::ProfileConstants::Section, value.profiles},
        {DisplayConstants::Preference, value.interfaceScale},
        {AppConstants::DefaultImageDirectory, value.defaultImageDirectory},
        {FileDialogPreferences::JsonKey, value.fileDialogDirectories.value_or(QJsonObject{})},
        {EmulationShortcutsKey, value.emulationShortcuts},
        {StorageDirectoryKey, value.emulationFolders.storage},
        {CaptureDirectoryKey, value.emulationFolders.captures},
        {StateDirectoryKey, value.emulationFolders.states},
        {FullscreenStatisticsKey, value.showFullscreenEmulationStatistics}
    }).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error(ErrorKeys::CannotSavePreferences);
}
UserPreferences PreferencesStore::loadOrCreate(const UserPreferences& defaults) const {
    const bool existed = QFileInfo::exists(path_);
    auto selected = load(defaults);
    const bool needsMigration = !selected.fileDialogDirectories.has_value();
    if (needsMigration) {
        QJsonObject directories;
        QSettings settings;
        settings.beginGroup(FileDialogPreferences::LegacySettingsGroup);
        for (const auto& key : settings.childKeys()) {
            const auto path = settings.value(key).toString();
            if (!key.isEmpty() && !path.contains(QChar::Null)) directories.insert(key, path);
        }
        selected.fileDialogDirectories = std::move(directories);
    }
    if (!existed || needsMigration) save(selected);
    return selected;
}
}
