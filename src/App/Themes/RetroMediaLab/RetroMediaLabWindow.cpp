#include "RetroMediaLabWindow.h"
#include "RetroMediaLabLayout.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Platform/WindowAppearance.h>
#include <QApplication>
#include <QCloseEvent>
#include <QIcon>
#include <QScreen>
namespace RetroMediaLab {
RetroMediaLabWindow::RetroMediaLabWindow(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state)
    : state_(state) {
    setObjectName("RetroMediaLabWindow");
    setWindowTitle(QString::fromStdString(translations.text("App.Title")));
    setWindowIcon(QIcon(QApplication::applicationDirPath() + "/Assets/app-icon.ico"));
    setCentralWidget(new RetroMediaLabLayout(themes, translations, this));
    const auto available = screen()->availableGeometry();
    const auto room = (available.size() - QSize(48, 48)).expandedTo(QSize(1, 1));
    setMinimumSize(qMin(800, room.width()), qMin(480, room.height()));
    resize(qMin(1360, room.width()), qMin(820, room.height()));
    move(available.center() - rect().center());
    if (state.hasGeometry()) restoreGeometry(state.mainWindowGeometry);
    if (state.mainWindowMaximized) setWindowState(windowState() | Qt::WindowMaximized);
    const auto native = static_cast<std::uintptr_t>(winId());
    Platform::applyWindowAppearance(native, themes.variant() == ThemeVariant::Dark);
    themeSubscription_ = themes.subscribe([this, &themes] {
        Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes.variant() == ThemeVariant::Dark);
    });
    languageSubscription_ = translations.subscribe([this, &translations] {
        setWindowTitle(QString::fromStdString(translations.text("App.Title")));
    });
}
void RetroMediaLabWindow::synchronizeState() {
    state_.mainWindowGeometry = saveGeometry();
    state_.mainWindowMaximized = isMaximized();
}
void RetroMediaLabWindow::closeEvent(QCloseEvent* event) {
    synchronizeState();
    state_.mainWindowOpen = false;
    event->accept();
    QApplication::quit();
}
}
