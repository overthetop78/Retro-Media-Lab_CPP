#include "../ThemeConstants.h"
#include "StatusBar.h"
#include "Shared/Controls/StatusIndicator.h"
#include "../ThemeIcons.h"
#include "Maintenance/GreaseweazleTools.h"
#include "Maintenance/GwToolsConstants.h"
#include "Media/ConversionSession.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QComboBox>
#include <QMap>
#include <QSignalBlocker>
#include <algorithm>
namespace RetroMediaLab {
StatusBar::StatusBar(ThemeService& themes, Translations::TranslationService& translations,
    WorkspaceState& state, bool expanded, std::function<void(bool)> toggle, QWidget* parent) : QWidget(parent) {
    setObjectName(ThemeConstants::StatusBar);
    setMinimumHeight(ThemeConstants::StatusBarLayout::BarHeight);
    setAutoFillBackground(true);
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset, ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset);
    auto* hardware = new StatusIndicator(this);
    hardware->setObjectName(ThemeConstants::HardwareIndicator);
    hardware->setMaximumWidth(ThemeConstants::StatusBarLayout::HardwareMaximumWidth);
    auto* selector = new QComboBox(this);
    selector->setObjectName(ThemeConstants::HardwareSelector);
    selector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    selector->setMaximumWidth(ThemeConstants::StatusBarLayout::HardwareMaximumWidth);
    selector->setFixedHeight(ThemeConstants::StatusBarLayout::BarHeight - ThemeConstants::StatusBarLayout::CombinedVerticalInset);
    auto* ready = new QLabel(this);
    ready->setObjectName(ThemeConstants::ReadyStatus);
    auto* console = new QToolButton(this);
    console->setObjectName(ThemeConstants::ToggleConsole);
    console->setCheckable(true);
    console->setChecked(expanded);
    layout->addWidget(hardware);
    layout->addWidget(selector);
    layout->addSpacing(ThemeConstants::StatusBarLayout::ToggleExtent);
    layout->addWidget(ready);
    layout->addStretch();
    layout->addWidget(console);
    connect(console, &QToolButton::toggled, this, [toggle = std::move(toggle)](bool checked) {
        toggle(checked);
    });
    const auto updateSelection = [hardware, selector, &state, &themes, &translations] {
        const auto id = selector->currentData().toString();
        state.selectedHardwareDeviceId = id;
        const auto& devices = state.hardware().devices;
        const auto selected = std::find_if(devices.cbegin(), devices.cend(), [&](const auto& device) { return device.id == id; });
        const bool multiple = selector->count() > 1;
        selector->setVisible(multiple);
        selector->setToolTip(selector->currentText());
        selector->setAccessibleName(QString::fromStdString(translations.text(HardwareConstants::TitleKey)));
        hardware->setStatus(multiple ? QString() : selector->count() ? selector->currentText()
            : QString::fromStdString(translations.text(ThemeConstants::HardwareNotConfigured)),
            themes.data()->color(selected != devices.cend() && selected->available ? HardwareConstants::SuccessBrush : ThemeConstants::MutedTextBrush));
        hardware->setToolTip(selector->count() ? selector->currentText() : QString());
    };
    connect(selector, &QComboBox::currentIndexChanged, this, [updateSelection] { updateSelection(); });
    const auto refreshOperation=[ready,&state,&translations] {
        const auto active=state.converting && state.converting->converting()?QString::fromLatin1(GwToolsConstants::Convert):state.greaseweazleTools?state.greaseweazleTools->activeOperation():QString{};
        const auto* key=active==EngineConstants::Read?AppConstants::TabRead:active==EngineConstants::Write?AppConstants::TabWrite:active==GwToolsConstants::Convert?AppConstants::TabConvert:active.isEmpty()?ThemeConstants::StatusReadyShort:GwToolsConstants::RunningKey;
        ready->setText(QString::fromStdString(translations.text(key)));
    };
    const auto refresh = [this, refreshOperation, console, selector, updateSelection, &state, &themes, &translations] {
        const QSignalBlocker blocked(selector);
        selector->clear();
        QMap<QString, QMap<QString, int>> numbers;
        for (const auto& device : state.hardware().devices) {
            if (device.id.isEmpty()) continue;
            QString label;
            if (device.category == HardwareConstants::Floppy) {
                const auto number = ++numbers[device.backend][device.controller.toLower()];
                const auto drive = device.backend == HardwareConstants::KryoFlux ? device.selection : QString::number(number);
                label = QString::fromStdString(translations.text(ThemeConstants::HardwareDriveChoice));
                label.replace(ThemeConstants::HardwareField0, drive).replace(ThemeConstants::HardwareField1, device.size)
                    .replace(ThemeConstants::HardwareField2, device.density == HardwareConstants::UnknownValue
                        ? QString::fromStdString(translations.text(HardwareConstants::UnknownKey)) : device.density)
                    .replace(ThemeConstants::HardwareField3, device.backend + ThemeConstants::HardwareDetailSeparator + device.address);
            } else {
                label = device.name;
            }
            if (!device.available) label = QString::fromUtf8(ThemeConstants::HardwareDisconnectedFormat)
                .arg(label, QString::fromStdString(translations.text(ThemeConstants::HardwareDisconnected)));
            selector->addItem(label, device.id);
        }
        const auto previous = selector->findData(state.selectedHardwareDeviceId);
        selector->setCurrentIndex(previous >= 0 ? previous : selector->count() ? 0 : -1);
        updateSelection();
        refreshOperation();
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
    hardwareSubscription_ = state.subscribeToHardware(refresh);
    if(state.greaseweazleTools)operationSubscription_=state.greaseweazleTools->subscribe(refreshOperation);
    if(state.converting)conversionSubscription_=state.converting->subscribe(refreshOperation);
}
}

