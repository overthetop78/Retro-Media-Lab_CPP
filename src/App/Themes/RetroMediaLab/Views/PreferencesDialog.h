#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QDialog>
class QComboBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QTabWidget;
class QLineEdit;
class QFileDialog;
namespace RetroMediaLab {
class ThemeSettingsPage;
class PreferencesDialog final : public QDialog {
public:
    PreferencesDialog(ThemeService&, Translations::TranslationService&, WorkspaceState&, QWidget*);
    ~PreferencesDialog() override;
    void synchronizeState();
    void done(int) override;
protected:
    void showEvent(QShowEvent*) override;
private:
    void refreshTexts();
    void refreshAppearance();
    void browseDirectory();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    QTabWidget* sections_;
    QComboBox* languages_;
    QLineEdit* directory_;
    QLabel* directoryLabel_;
    QPushButton* browse_;
    std::unique_ptr<QFileDialog> directoryPicker_;
    QLabel* languageLabel_;
    QPushButton* close_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
};
}
