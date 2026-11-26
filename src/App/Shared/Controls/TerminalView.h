#pragma once
#include "AppApi.h"
#include "Shell/ConsoleState.h"
#include "Shared/Presentation/ThemeService.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QWidget>
#include <QIcon>
class QPlainTextEdit;
class QPushButton;
class QLabel;
namespace RetroMediaLab {
class TerminalView final : public QWidget {
public:
    RETROMEDIALAB_APP_API TerminalView(ThemeService&, Translations::TranslationService&, ConsoleState&, QWidget*);
    RETROMEDIALAB_APP_API void synchronizeState();
    RETROMEDIALAB_APP_API void restoreScroll();
    RETROMEDIALAB_APP_API void setCopyIcon(const QIcon&);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    ConsoleState& state_;
    ThemeService& themes_;
    QPlainTextEdit *command_, *output_;
    QPushButton* copy_;
    QLabel* prompt_;
    QString initialStatus_;
    Translations::TranslationService::Subscription languageSubscription_;
    ThemeService::Subscription themeSubscription_;
    ConsoleState::Subscription outputSubscription_;
};
}
