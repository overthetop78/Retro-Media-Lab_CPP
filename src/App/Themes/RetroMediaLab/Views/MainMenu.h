#pragma once
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QMenuBar>
#include <functional>
namespace RetroMediaLab {
struct WorkspaceState;
class MainMenu final : public QMenuBar {
public:
    MainMenu(Translations::TranslationService&, std::function<void()> openPreferences, std::function<void()> openGreaseweazle, std::function<void()> openEmulationSettings, WorkspaceState&,
        std::function<void(const QString&)> openModuleOptions, QWidget*);
};
}
