#pragma once
#include "State/WorkspaceState.h"
#include "Shared/Presentation/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QTabWidget>
namespace RetroMediaLab {
class WorkspaceTabs final : public QTabWidget {
public:
    WorkspaceTabs(ThemeService&, Translations::TranslationService&, WorkspaceState&, std::function<void()>, QWidget*);
    void openEmulationSettings();
    void openEmulationModuleOptions(const QString&);
private:
    Translations::TranslationService::Subscription subscription_;
    ThemeService::Subscription themeSubscription_;
};
}
