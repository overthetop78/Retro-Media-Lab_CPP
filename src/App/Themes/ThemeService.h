#pragma once
#include <Constants/AppConstants.h>
#include "AppApi.h"
#include <QColor>
#include <QJsonObject>
#include <QMap>
#include <QPalette>
#include <QString>
#include <functional>
#include <memory>
#include <vector>
class QStyle;
class QPalette;
namespace RetroMediaLab {
struct ThemeModule;
using ThemeModuleHandle = std::shared_ptr<ThemeModule>;
class IThemePresentation;
struct WorkspaceState;
namespace Translations { class TranslationService; }
struct ThemeData {
    QMap<QString, QColor> colors;
    int fontSize = AppConstants::Service::DefaultFontSize, controlHeight = AppConstants::Service::DefaultControlHeight, cardPadding = AppConstants::Service::DefaultCardPadding, cardRadius = AppConstants::Service::DefaultCardRadius, controlRadius = AppConstants::Service::DefaultControlRadius;
    int tabPaddingX = AppConstants::Service::DefaultTabPaddingX, tabPaddingY = AppConstants::Service::DefaultTabPaddingY, tabRadius = AppConstants::Service::DefaultTabRadius;
    QString fontFamily;
    bool dark = false, nativeSystemColors = false;
    bool operator==(const ThemeData&) const = default;
    RETROMEDIALAB_APP_API QColor color(const QString& key) const;
};
class ThemeService final {
public:
    using Subscription = std::shared_ptr<const std::function<void()>>;
    RETROMEDIALAB_APP_API static ThemeModuleHandle loadModule(const QString&);
    RETROMEDIALAB_APP_API explicit ThemeService(ThemeModuleHandle);
    RETROMEDIALAB_APP_API ~ThemeService();
    ThemeService(const ThemeService&) = delete;
    ThemeService& operator=(const ThemeService&) = delete;
    RETROMEDIALAB_APP_API std::unique_ptr<IThemePresentation> createPresentation(Translations::TranslationService&, WorkspaceState&);
    RETROMEDIALAB_APP_API QStyle* createStyle() const;
    RETROMEDIALAB_APP_API void applySettings(const QJsonObject&);
    RETROMEDIALAB_APP_API std::unique_ptr<ThemeService> createPreview(const QJsonObject&) const;
    bool isPreview() const noexcept { return preview_; }
    const QPalette& palette() const noexcept { return palette_; }
    const QPalette& systemPalette() const noexcept { return systemPalette_; }
    void rememberSystemPalette(const QPalette& palette) { systemPalette_ = palette; }
    const QJsonObject& settings() const noexcept { return settings_; }
    const QString& id() const noexcept { return id_; }
    std::shared_ptr<const ThemeData> data() const { return data_; }
    RETROMEDIALAB_APP_API Subscription subscribe(std::function<void()> observer);
    // Called by the loaded theme after validating its own settings. A null palette
    // leaves the platform palette active (for themes that choose system colors).
    RETROMEDIALAB_APP_API void publish(const ThemeData&, const QJsonObject&, const QPalette* palette = nullptr);
private:
    RETROMEDIALAB_APP_API ThemeService(ThemeModuleHandle, bool preview, const QPalette& systemPalette);
    struct Configuration;
    ThemeModuleHandle module_; // Must outlive theme-owned configuration and views.
    QString id_;
    std::shared_ptr<ThemeData> data_;
    QJsonObject settings_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
    bool preview_ = false;
    QPalette systemPalette_, palette_;
    std::unique_ptr<Configuration> configuration_;
};
}
