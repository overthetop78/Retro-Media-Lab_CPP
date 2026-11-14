#include "../ThemeConstants.h"
#include "StatusBar.h"
#include "Shared/Controls/StatusIndicator.h"
#include "../ThemeIcons.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
namespace RetroMediaLab {
StatusBar::StatusBar(ThemeService& themes, Translations::TranslationService& translations,
    bool expanded, std::function<void(bool)> toggle, QWidget* parent) : QWidget(parent) {
    setObjectName(ThemeConstants::StatusBar);
    setMinimumHeight(ThemeConstants::StatusBarLayout::BarHeight);
    setAutoFillBackground(true);
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset, ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset);
    auto* hardware = new StatusIndicator(this);
    auto* ready = new QLabel(this);
    ready->setObjectName(ThemeConstants::ReadyStatus);
    auto* console = new QToolButton(this);
    console->setObjectName(ThemeConstants::ToggleConsole);
    console->setCheckable(true);
    console->setChecked(expanded);
    layout->addWidget(hardware);
    layout->addSpacing(ThemeConstants::StatusBarLayout::ToggleExtent);
    layout->addWidget(ready);
    layout->addStretch();
    layout->addWidget(console);
    connect(console, &QToolButton::toggled, this, [toggle = std::move(toggle)](bool checked) {
        toggle(checked);
    });
    const auto refresh = [this, hardware, ready, console, &themes, &translations] {
        hardware->setStatus(QString::fromStdString(translations.text(ThemeConstants::HardwareNotConfigured)),
            themes.data()->color(ThemeConstants::MutedTextBrush));
        ready->setText(QString::fromStdString(translations.text(ThemeConstants::StatusReadyShort)));
        const auto label = QString::fromStdString(translations.text(ThemeConstants::CommonConsole));
        console->setToolTip(label);
        console->setAccessibleName(label);
        console->setIcon(themeIcon(ThemeConstants::Console, themes.data()->color(ThemeConstants::MutedTextBrush)));
        console->setIconSize(QSize(ThemeConstants::StatusBarLayout::ToggleIconSize, ThemeConstants::StatusBarLayout::ToggleIconSize));
        auto colors = palette();
        colors.setColor(QPalette::Window, themes.data()->color(ThemeConstants::StatusBrush));
        setPalette(colors);
    };
    refresh();
    languageSubscription_ = translations.subscribe(refresh);
    themeSubscription_ = themes.subscribe(refresh);
}
}

