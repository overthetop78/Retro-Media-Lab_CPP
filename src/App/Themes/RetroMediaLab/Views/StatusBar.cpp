#include "StatusBar.h"
#include "Shared/Controls/StatusIndicator.h"
#include "../ThemeIcons.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
namespace RetroMediaLab {
StatusBar::StatusBar(ThemeService& themes, Translations::TranslationService& translations,
    bool expanded, std::function<void(bool)> toggle, QWidget* parent) : QWidget(parent) {
    setObjectName("statusBar");
    setMinimumHeight(30);
    setAutoFillBackground(true);
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 2, 8, 2);
    auto* hardware = new StatusIndicator(this);
    auto* ready = new QLabel(this);
    ready->setObjectName("readyStatus");
    auto* console = new QToolButton(this);
    console->setObjectName("toggleConsole");
    console->setCheckable(true);
    console->setChecked(expanded);
    layout->addWidget(hardware);
    layout->addSpacing(22);
    layout->addWidget(ready);
    layout->addStretch();
    layout->addWidget(console);
    connect(console, &QToolButton::toggled, this, [toggle = std::move(toggle)](bool checked) {
        toggle(checked);
    });
    const auto refresh = [this, hardware, ready, console, &themes, &translations] {
        hardware->setStatus(QString::fromStdString(translations.text("Hardware.NotConfigured")),
            themes.data()->color("MutedTextBrush"));
        ready->setText(QString::fromStdString(translations.text("Status.ReadyShort")));
        const auto label = QString::fromStdString(translations.text("Common.Console"));
        console->setToolTip(label);
        console->setAccessibleName(label);
        console->setIcon(themeIcon("console", themes.data()->color("MutedTextBrush")));
        console->setIconSize(QSize(20, 20));
        auto colors = palette();
        colors.setColor(QPalette::Window, themes.data()->color("StatusBrush"));
        setPalette(colors);
    };
    refresh();
    languageSubscription_ = translations.subscribe(refresh);
    themeSubscription_ = themes.subscribe(refresh);
}
}

