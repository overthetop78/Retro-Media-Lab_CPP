#pragma once
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QMenuBar>
#include <functional>
namespace RetroMediaLab {
class MainMenu final : public QMenuBar {
public:
    MainMenu(Translations::TranslationService&, std::function<void()> openPreferences, std::function<void()> openGreaseweazle, QWidget*);
};
}
