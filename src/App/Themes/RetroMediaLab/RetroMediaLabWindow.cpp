#include "RetroMediaLabWindow.h"
#include "RetroMediaLabLayout.h"
#include "State/WorkspaceState.h"
#include "Views/PreferencesDialog.h"
#include <RetroMediaLab/Platform/WindowAppearance.h>
#include <QApplication>
#include <QCloseEvent>
#include <QIcon>
#include <QScreen>
#include <QTimer>
namespace RetroMediaLab {
RetroMediaLabWindow::RetroMediaLabWindow(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state)
    : themes_(themes), translations_(translations), state_(state) {
    setObjectName("RetroMediaLabWindow");
    setWindowTitle(QString::fromStdString(translations.text("App.Title")));
    setWindowIcon(QIcon(QApplication::applicationDirPath() + "/Assets/app-icon.ico"));
    setCentralWidget(new RetroMediaLabLayout(themes, translations, state, [this] { openPreferences(); }, this));
    const auto available = screen()->availableGeometry();
    const auto room = (available.size() - QSize(48, 48)).expandedTo(QSize(1, 1));
    setMinimumSize(qMin(800, room.width()), qMin(480, room.height()));
    resize(qMin(1360, room.width()), qMin(820, room.height()));
    move(available.center() - rect().center());
    if (state.hasGeometry()) restoreGeometry(state.mainWindowGeometry);
    if (state.mainWindowMaximized) setWindowState(windowState() | Qt::WindowMaximized);
    const auto native = static_cast<std::uintptr_t>(winId());
    Platform::applyWindowAppearance(native, themes.variant() == ThemeVariant::Dark,
        themes.data()->color("CaptionBrush").rgba(), themes.data()->color("TextBrush").rgba());
    themeSubscription_ = themes.subscribe([this, &themes] {
        Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes.variant() == ThemeVariant::Dark,
            themes.data()->color("CaptionBrush").rgba(), themes.data()->color("TextBrush").rgba());
    });
    languageSubscription_ = translations.subscribe([this, &translations] {
        setWindowTitle(QString::fromStdString(translations.text("App.Title")));
    });
}
RetroMediaLabWindow::~RetroMediaLabWindow() = default;
void RetroMediaLabWindow::openPreferences() {
    if (!preferences_) preferences_ = std::make_unique<PreferencesDialog>(themes_, translations_, state_, this);
    state_.preferencesOpen = true;
    preferences_->show();
    preferences_->raise();
    preferences_->activateWindow();
}
void RetroMediaLabWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    // Qt can reset native frame attributes while mapping the window.
    // The context-bound callback is cancelled automatically if this view is destroyed.
    QTimer::singleShot(0, this, [this] {
        Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes_.variant() == ThemeVariant::Dark,
            themes_.data()->color("CaptionBrush").rgba(), themes_.data()->color("TextBrush").rgba());
    });
    if (state_.preferencesOpen) openPreferences();
}
void RetroMediaLabWindow::synchronizeState() {
    if (preferences_) preferences_->synchronizeState();
    static_cast<RetroMediaLabLayout*>(centralWidget())->synchronizeState();
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
