#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QTabWidget;
class QLabel;
class QComboBox;
namespace RetroMediaLab {
class EngineSettingsPage final : public QWidget {
public:
    EngineSettingsPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
private:
    void refreshTexts();
    struct Row {QString operation;const char* labelKey;QLabel* label;QComboBox* choice;};
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QTabWidget* backends_;
    QLabel *description_,*strict_;
    QVector<Row> rows_;
    Translations::TranslationService::Subscription languageSubscription_;
    WorkspaceState::DirectorySubscription settingsSubscription_;
};
}
