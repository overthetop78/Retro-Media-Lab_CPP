#include "ThemeConstants.h"
#include "RetroMediaLabLayout.h"
#include "Shared/Controls/TerminalView.h"
#include "ThemeIcons.h"
#include "Views/MainMenu.h"
#include "Views/WorkspaceTabs.h"
#include "Views/StatusBar.h"
#include <QSplitter>
#include <QVBoxLayout>
namespace RetroMediaLab {
RetroMediaLabLayout::RetroMediaLabLayout(ThemeService& themes, Translations::TranslationService& translations,
    WorkspaceState& state, std::function<void()> openPreferences, std::function<void()> openGreaseweazle, QWidget* parent)
    : QWidget(parent), state_(state) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(new MainMenu(translations, std::move(openPreferences), std::move(openGreaseweazle), this));
    auto* area = new QWidget(this);
    auto* areaLayout = new QVBoxLayout(area);
    areaLayout->setContentsMargins(ThemeConstants::Layout::ConsoleToggleIconSize, ThemeConstants::Layout::PagePadding, ThemeConstants::Layout::ConsoleToggleIconSize, ThemeConstants::Layout::ContentSpacing);
    splitter_ = new QSplitter(Qt::Vertical, area);
    splitter_->setObjectName(ThemeConstants::WorkspaceSplitter);
    splitter_->setHandleWidth(ThemeConstants::Layout::OuterSpacing);
    splitter_->setChildrenCollapsible(false);
    auto* pages = new WorkspaceTabs(themes, translations, state, splitter_);
    terminal_ = new TerminalView(themes, translations, state.console, splitter_);
    const auto refreshIcon = [this, &themes] {
        terminal_->setCopyIcon(themeIcon(ThemeConstants::Copy, themes.data()->color(ThemeConstants::TerminalText)));
    };
    refreshIcon();
    themeSubscription_ = themes.subscribe(refreshIcon);
    terminal_->setMinimumHeight(ThemeConstants::Layout::MinimumContentHeight);
    pages->setMinimumHeight(ThemeConstants::Layout::MinimumConsoleHeight);
    splitter_->addWidget(pages);
    splitter_->addWidget(terminal_);
    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 0);
    terminal_->setVisible(state.console.expanded);
    connect(splitter_, &QSplitter::splitterMoved, this, [this] {
        if (state_.console.expanded)
            state_.console.height = splitter_->sizes().value(1, state_.console.height);
    });
    areaLayout->addWidget(splitter_);
    layout->addWidget(area, 1);
    layout->addWidget(new StatusBar(themes, translations, state, state.console.expanded, [this](bool expanded) {
        if (!expanded) state_.console.height = splitter_->sizes().value(1, state_.console.height);
        state_.console.expanded = expanded;
        terminal_->setVisible(expanded);
        if (expanded) {
            splitter_->setSizes({qMax(ThemeConstants::Layout::MinimumConsoleHeight, splitter_->height() - state_.console.height), state_.console.height});
            terminal_->restoreScroll();
        }
    }, this));
}
void RetroMediaLabLayout::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!restored_) {
        restored_ = true;
        splitter_->setSizes({qMax(ThemeConstants::Layout::MinimumConsoleHeight, splitter_->height() - state_.console.height), state_.console.height});
        terminal_->restoreScroll();
    }
}
void RetroMediaLabLayout::synchronizeState() {
    if (state_.console.expanded)
        state_.console.height = splitter_->sizes().value(1, state_.console.height);
    terminal_->synchronizeState();
}
}
