#pragma once
#include "../ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QMainWindow>
namespace RetroMediaLab {
struct WorkspaceState;
class PreferencesDialog;
class DisplayDensity;
class RetroMediaLabWindow final : public QMainWindow {
public:
    RetroMediaLabWindow(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state);
    ~RetroMediaLabWindow() override;
    void synchronizeState();
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent*) override;
private:
    void openPreferences();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::unique_ptr<PreferencesDialog> preferences_;
    std::unique_ptr<DisplayDensity> density_;
    ThemeService::Subscription themeSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
