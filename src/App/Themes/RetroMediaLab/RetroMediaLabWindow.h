#pragma once
#include "../ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QMainWindow>
namespace RetroMediaLab {
struct WorkspaceState;
class RetroMediaLabWindow final : public QMainWindow {
public:
    RetroMediaLabWindow(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state);
    void synchronizeState();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    WorkspaceState& state_;
    ThemeService::Subscription themeSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
