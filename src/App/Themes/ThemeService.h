#pragma once
#include "AppApi.h"
#include <QColor>
#include <QMap>
#include <QString>
#include <functional>
#include <memory>
#include <vector>
class QStyle;
namespace RetroMediaLab {
struct ThemeModule;
using ThemeModuleHandle = std::shared_ptr<ThemeModule>;
class IThemePresentation;
struct WorkspaceState;
namespace Translations { class TranslationService; }
enum class ThemeVariant { Light, Dark };
struct ThemeData {
    QMap<QString, QColor> colors;
    int fontSize = 14, controlHeight = 36, cardPadding = 18, cardRadius = 8, controlRadius = 4;
    bool dark = false;
    RETROMEDIALAB_APP_API QColor color(const QString& key) const;
};
class ThemeService final {
public:
    using Subscription = std::shared_ptr<const std::function<void()>>;
    RETROMEDIALAB_APP_API static ThemeModuleHandle loadModule(const QString& path);
    RETROMEDIALAB_APP_API explicit ThemeService(ThemeModuleHandle module);
    RETROMEDIALAB_APP_API std::unique_ptr<IThemePresentation> createPresentation(Translations::TranslationService&, WorkspaceState&);
    RETROMEDIALAB_APP_API QStyle* createStyle() const;
    RETROMEDIALAB_APP_API void setVariant(ThemeVariant variant);
    RETROMEDIALAB_APP_API ThemeVariant variant() const noexcept;
    std::shared_ptr<const ThemeData> data() const { return data_; }
    RETROMEDIALAB_APP_API Subscription subscribe(std::function<void()> observer);
private:
    ThemeModuleHandle module_;
    QString directory_;
    std::shared_ptr<ThemeData> data_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
};
}
