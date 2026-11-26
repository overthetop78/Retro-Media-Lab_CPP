#pragma once
#include "Shared/Presentation/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QMainWindow>
namespace RetroMediaLab {
struct WorkspaceState;
class PreferencesDialog;
class MigrationDialog;
class GreaseweazleDialog;
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
    void openMigration();
    void openGreaseweazle();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::unique_ptr<PreferencesDialog> preferences_;
    std::unique_ptr<MigrationDialog> migration_;
    std::unique_ptr<GreaseweazleDialog> greaseweazle_;
    std::unique_ptr<DisplayDensity> density_;
    ThemeService::Subscription themeSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
