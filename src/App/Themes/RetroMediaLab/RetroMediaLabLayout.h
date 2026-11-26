#pragma once
#include "Shared/Presentation/ThemeService.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
class QSplitter;
namespace RetroMediaLab {
class TerminalView;
class RetroMediaLabLayout final : public QWidget {
public:
    RetroMediaLabLayout(ThemeService&, Translations::TranslationService&, WorkspaceState&, std::function<void()>, std::function<void()>, std::function<void()>, QWidget*);
    void synchronizeState();
protected:
    void showEvent(QShowEvent*) override;
private:
    WorkspaceState& state_;
    QSplitter* splitter_;
    TerminalView* terminal_;
    bool restored_ = false;
    ThemeService::Subscription themeSubscription_;
};
}
