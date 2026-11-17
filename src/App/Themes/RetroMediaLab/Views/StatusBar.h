#pragma once
#include "Themes/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <functional>
namespace RetroMediaLab {
class StatusBar final : public QWidget {
public:
    StatusBar(ThemeService&, Translations::TranslationService&, WorkspaceState&, bool, std::function<void(bool)>, QWidget*);
private:
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
    WorkspaceState::DirectorySubscription hardwareSubscription_;
    WorkspaceState::DirectorySubscription operationSubscription_;
};
}
