#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QDialog>
#include "Maintenance/GreaseweazleTools.h"
class QStackedWidget;
class QListWidget;
class QPushButton;
class QComboBox;
class QLabel;
namespace RetroMediaLab {
class GreaseweazleDialog final : public QDialog {
public:
    GreaseweazleDialog(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
    void synchronizeState();
    void done(int) override;
protected:
    void showEvent(QShowEvent*) override;
private:
    void refreshTexts();
    void refreshAppearance();
    void refreshReaders();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QStackedWidget* pages_;
    QListWidget* navigation_;
    QPushButton* close_;
    QComboBox* readers_;
    QLabel* readerLabel_;
    bool closeRequested_ = false;
    WorkspaceState::DirectorySubscription hardwareSubscription_;
    GreaseweazleTools::Subscription toolsSubscription_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
};
}
