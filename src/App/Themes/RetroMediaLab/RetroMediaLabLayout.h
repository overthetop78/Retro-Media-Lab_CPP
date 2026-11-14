#pragma once
#include "../ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QLabel;
class QComboBox;
namespace RetroMediaLab {
class RetroMediaLabLayout final : public QWidget {
public:
    RetroMediaLabLayout(ThemeService& themes, Translations::TranslationService& translations, QWidget* parent);
private:
    void refreshTexts();
    void refreshColors();
    ThemeService& themes_;
    Translations::TranslationService& translations_;
    QLabel *themeLabel_, *languageLabel_, *welcome_, *description_, *ready_;
    QComboBox *variants_, *languages_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
};
}
