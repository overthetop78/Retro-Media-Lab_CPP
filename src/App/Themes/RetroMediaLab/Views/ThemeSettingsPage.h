#pragma once
#include "Themes/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;
namespace RetroMediaLab {
class ThemePreview;
class ThemeSettingsPage final : public QWidget {
public:
    ThemeSettingsPage(ThemeService&, Translations::TranslationService&, const WorkspaceState&, QWidget* previewSource, QWidget* parent);
private:
    void refreshTexts();
    void refreshAppearance();
    void refreshSelection();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    QListWidget* available_;
    QLabel *availableTitle_, *listName_, *variants_, *name_, *description_, *appearance_, *active_, *persistence_;
    QRadioButton *system_, *light_, *dark_;
    QPushButton* choose_;
    QString pendingAppearance_;
    ThemePreview* preview_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
};
}
