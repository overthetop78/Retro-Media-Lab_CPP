#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <memory>
#include <vector>
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QMessageBox;
namespace RetroMediaLab {
class LogSettingsPage final : public QWidget {
public:
    LogSettingsPage(ThemeService&,Translations::TranslationService&,WorkspaceState&,QWidget*);
    ~LogSettingsPage() override;
    void commitEdits();
protected:
    bool eventFilter(QObject*,QEvent*) override;
private:
    struct Row {QString action;const char* labelKey;QLabel *label,*maximumLabel,*unit;QCheckBox *enabled,*archives;QLineEdit* maximum;};
    void refreshTexts();
    void refreshSettings();
    void commitMaximum(Row&);
    void openFolder();
    Translations::TranslationService& translations_;
    WorkspaceState& state_;
    std::vector<Row> rows_;
    QLabel *description_,*hint_,*directory_;
    QPushButton* open_;
    std::unique_ptr<QMessageBox> error_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
    WorkspaceState::LoggingSubscription settingsSubscription_;
};
}
