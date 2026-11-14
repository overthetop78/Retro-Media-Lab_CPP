#include "ThemeConstants.h"
#include "RetroMediaLabWindow.h"
#include "RetroMediaLabLayout.h"
#include "DisplayDensity.h"
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
    density_ = std::make_unique<DisplayDensity>(*this, state);
    setObjectName(ThemeConstants::RetroMediaLabWindow);
    setWindowTitle(QString::fromStdString(translations.text(ThemeConstants::AppTitle)));
    setWindowIcon(QIcon(QApplication::applicationDirPath() + ThemeConstants::AssetsAppIconIco));
    setCentralWidget(new RetroMediaLabLayout(themes, translations, state, [this] { openPreferences(); }, this));
    const auto available = screen()->availableGeometry();
    const auto room = (available.size() - QSize(ThemeConstants::Window::ScreenInset, ThemeConstants::Window::ScreenInset)).expandedTo(QSize(1, 1));
    setMinimumSize(qMin(ThemeConstants::Window::MinimumWidth, room.width()), qMin(ThemeConstants::Window::MinimumHeight, room.height()));
    resize(qMin(ThemeConstants::Window::DefaultWidth, room.width()), qMin(ThemeConstants::Window::DefaultHeight, room.height()));
    move(available.center() - rect().center());
    if (state.hasGeometry()) restoreGeometry(state.mainWindowGeometry);
    if (state.mainWindowMaximized) setWindowState(windowState() | Qt::WindowMaximized);
    const auto native = static_cast<std::uintptr_t>(winId());
    Platform::applyWindowAppearance(native, themes.data()->dark,
        themes.data()->color(ThemeConstants::CaptionBrush).rgba(), themes.data()->color(ThemeConstants::TextBrush).rgba(), themes.data()->nativeSystemColors);
    themeSubscription_ = themes.subscribe([this, &themes] {
        Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes.data()->dark,
            themes.data()->color(ThemeConstants::CaptionBrush).rgba(), themes.data()->color(ThemeConstants::TextBrush).rgba(), themes.data()->nativeSystemColors);
    });
    languageSubscription_ = translations.subscribe([this, &translations] {
        setWindowTitle(QString::fromStdString(translations.text(ThemeConstants::AppTitle)));
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
        Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes_.data()->dark,
            themes_.data()->color(ThemeConstants::CaptionBrush).rgba(), themes_.data()->color(ThemeConstants::TextBrush).rgba(), themes_.data()->nativeSystemColors);
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
