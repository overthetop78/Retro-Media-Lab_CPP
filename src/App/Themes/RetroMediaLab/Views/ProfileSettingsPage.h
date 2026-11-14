#pragma once
#include "Themes/ThemeService.h"
#include "../ThemeConstants.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <array>
class QLabel;
class QListWidget;
namespace RetroMediaLab {
class ProfileSettingsPage final : public QWidget {
public:
    ProfileSettingsPage(ThemeService&, Translations::TranslationService&, QWidget*);
private:
    void refreshTexts();
    Translations::TranslationService& translations_;
    std::array<QLabel*, ThemeConstants::ProfilesPage::CategoryCount> titles_;
    std::array<QListWidget*, ThemeConstants::ProfilesPage::CategoryCount> lists_;
    Translations::TranslationService::Subscription languageSubscription_;
};
}
