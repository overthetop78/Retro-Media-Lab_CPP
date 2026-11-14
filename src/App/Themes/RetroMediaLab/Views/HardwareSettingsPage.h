#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QTabWidget;
namespace RetroMediaLab {
class FloppySettingsPage;
class HardwareSettingsPage final : public QWidget {
public:
    HardwareSettingsPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
    void synchronizeState();
private:
    QTabWidget* categories_;
    FloppySettingsPage* floppy_;
    Translations::TranslationService::Subscription subscription_;
};
}
