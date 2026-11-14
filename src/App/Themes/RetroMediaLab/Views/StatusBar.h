#pragma once
#include "Themes/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <functional>
namespace RetroMediaLab {
class StatusBar final : public QWidget {
public:
    StatusBar(ThemeService&, Translations::TranslationService&, bool, std::function<void(bool)>, QWidget*);
private:
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
};
}
