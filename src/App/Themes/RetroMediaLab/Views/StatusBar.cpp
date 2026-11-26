#include "Media/MediaCommandConstants.h"
#include "../ThemeConstants.h"
#include "StatusBar.h"
#include "LoadingConstants.h"
#include "Media/VisualizationSession.h"
#include "Media/ExplorerReadSession.h"
#include "Shared/Localization/MediaText.h"
#include "Shared/Controls/StatusIndicator.h"
#include "../ThemeIcons.h"
#include "Maintenance/GreaseweazleTools.h"
#include "Maintenance/GwToolsConstants.h"
#include "Media/ConversionSession.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QTimer>
#include <QFontDatabase>
#include "CaptureProgressView.h"
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
    setFixedHeight(LoadingConstants::BarHeight);
    setAutoFillBackground(true);
    auto* layout = new QGridLayout(this);layout->setSpacing(0);
    layout->setContentsMargins(ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset, ThemeConstants::StatusBarLayout::HorizontalInset, ThemeConstants::StatusBarLayout::VerticalInset);
    auto* hardwareHost=new QWidget(this);hardwareHost->setFixedWidth(LoadingConstants::HardwareWidth);auto* hardwareLayout=new QHBoxLayout(hardwareHost);hardwareLayout->setContentsMargins(0,0,0,0);hardwareLayout->setSpacing(0);
    auto* hardware = new StatusIndicator(hardwareHost);
    hardware->setObjectName(ThemeConstants::HardwareIndicator);
    hardware->setMaximumWidth(ThemeConstants::StatusBarLayout::HardwareMaximumWidth);
    auto* selector = new QComboBox(this);
    selector->setObjectName(ThemeConstants::HardwareSelector);
    selector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    selector->setMaximumWidth(ThemeConstants::StatusBarLayout::HardwareMaximumWidth);
    selector->setFixedHeight(ThemeConstants::StatusBarLayout::BarHeight - ThemeConstants::StatusBarLayout::CombinedVerticalInset);
    auto* profileHost=new QWidget(this);profileHost->setFixedWidth(LoadingConstants::ProfileWidth);auto* profileLayout=new QHBoxLayout(profileHost);profileLayout->setContentsMargins(0,0,0,0);auto* profile=new QLabel(profileHost);profileLayout->addWidget(profile);profile->setObjectName(LoadingConstants::ProfileObject);profile->setFixedWidth(LoadingConstants::ProfileWidth);
    auto* ready = new StatusIndicator(this);
    ready->setObjectName(ThemeConstants::ReadyStatus);
    ready->setFixedWidth(LoadingConstants::OperationWidth);
    auto* tracks=new CaptureProgressView(themes,this,true);tracks->setObjectName(LoadingConstants::StatusBar);tracks->hide();auto* elapsed=new QLabel(this);elapsed->setObjectName(LoadingConstants::TimerObject);elapsed->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));elapsed->setContentsMargins(LoadingConstants::TimerMargin,0,LoadingConstants::TimerMargin,0);elapsed->hide();
    auto* console = new QToolButton(this);
    console->setObjectName(ThemeConstants::ToggleConsole);
    console->setCheckable(true);
    console->setChecked(expanded);
    hardwareLayout->addWidget(hardware);hardwareLayout->addWidget(selector,1);layout->addWidget(hardwareHost,0,0);layout->addWidget(profileHost,0,1);layout->addWidget(ready,0,LoadingConstants::OperationColumn);layout->addWidget(tracks,0,LoadingConstants::ProgressColumn);layout->setColumnStretch(LoadingConstants::ProgressColumn,1);layout->addWidget(elapsed,0,LoadingConstants::TimerColumn);layout->addWidget(console,0,LoadingConstants::ConsoleColumn);
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
    const auto refreshOperation=[ready,tracks,profile,elapsed,&state,&translations] {
        namespace L=LoadingConstants;std::optional<Infrastructure::LoadingProgress> progress;const char* operationKey=AppConstants::TabRead;Infrastructure::GwTrackProgress strip;bool running=false;qint64 milliseconds=0;
        if(state.reading&&state.reading->loading()){progress=state.reading->loadingProgress();strip=state.reading->execution().progress;running=!state.reading->preparingSource()&&!state.reading->summarizing();if(running)milliseconds=state.reading->elapsedMilliseconds();}
        else if(state.writing&&state.writing->loading()){progress=state.writing->loadingProgress();strip=state.writing->execution().progress;operationKey=AppConstants::TabWrite;running=!state.writing->preparingSource();if(running)milliseconds=state.writing->elapsedMilliseconds();}
        else if(state.explorerReading&&state.explorerReading->capture().loading()){auto& capture=state.explorerReading->capture();progress=capture.loadingProgress();strip=capture.execution().progress;running=capture.busy();if(running)milliseconds=capture.elapsedMilliseconds();}
        else if(state.explorer&&state.explorer->busy()){progress=state.explorer->progress();}
        else if(state.visualization&&state.visualization->preparing()){progress=state.visualization->presentationProgress();}
        else if(state.converting&&state.converting->busy()&&!state.converting->awaitingLosses()){progress=state.converting->loadingProgress();operationKey=AppConstants::TabConvert;running=state.converting->converting();if(running){milliseconds=state.converting->elapsedMilliseconds();if(state.greaseweazleTools->activeOperation()==GwToolsConstants::Convert)strip=state.greaseweazleTools->draft(GwToolsConstants::Convert).progress;}}
        const bool explorerPresentation = state.activePageId == AppConstants::Explorer && state.explorer &&
            (state.explorer->busy() || (state.visualization && state.visualization->preparing()));
        const bool showOverview = ((state.explorer && state.explorer->busy()) || (state.visualization && state.visualization->preparing())) && (state.activePageId == AppConstants::Visualize || explorerPresentation) && state.visualization &&
            !state.visualization->trackProgress().heads.empty() &&
            (!state.explorer->busy() || state.visualization->displayedFlux());
        if (progress && showOverview && !running) strip = state.visualization->trackProgress();
        QStringList labels;for(const auto head:strip.heads)labels<<mediaText(translations,{L::Face,{std::to_string(head)},{}});tracks->setProgress(strip,labels,progress.has_value());tracks->setVisible(progress.has_value()&&!strip.cylinders.empty()&&!strip.heads.empty());if(showOverview&&!running&&state.visualization&&state.visualization->presentingSegments()&&state.visualization->snapshot()->sequential)tracks->setSequential(*state.visualization->snapshot()->sequential,state.visualization->presentedSegments(),QString::fromStdString(translations.text(L::Tape)));if (progress && !running && state.visualization && !state.explorer->busy() &&
            (state.activePageId == AppConstants::Visualize || explorerPresentation) &&
            !state.visualization->linearProgress().empty()) {
            const auto& snapshot = state.visualization->snapshot();
            const bool optical = snapshot->descriptor->fields().representationKind == MediaEngine::RepresentationKind::OpticalTracks;
            tracks->setLinear(state.visualization->linearProgress(), QString::fromStdString(translations.text(optical ? L::Optical : L::Blocks)));
        }
        elapsed->setVisible(running);if(running){const auto seconds=milliseconds/L::MillisecondsPerSecond;elapsed->setText(QString::fromLatin1(L::TimePattern).arg(seconds/L::SecondsPerHour,L::TimeDigits,L::DecimalBase,QLatin1Char(L::Zero)).arg(seconds/L::SecondsPerMinute%L::SecondsPerMinute,L::TimeDigits,L::DecimalBase,QLatin1Char(L::Zero)).arg(seconds%L::SecondsPerMinute,L::TimeDigits,L::DecimalBase,QLatin1Char(L::Zero)));}
        const auto showProfile=[&](const auto& session){if(!session)return false;const auto& profiles=session->profiles();if(session->selectedProfile>=profiles.size())return false;const auto& selected=profiles[session->selectedProfile];const auto name=selected.system?QString::fromStdString(translations.text(L::DefaultProfile)):selected.name;const auto full=mediaText(translations,{L::Profile,{name.toStdString()},{}});profile->setText(profile->fontMetrics().elidedText(full,Qt::ElideRight,profile->width()));profile->setToolTip(full);return true;};
        bool hasProfile=false;if(state.activePageId==AppConstants::Read)hasProfile=showProfile(state.reading);else if(state.activePageId==AppConstants::Write)hasProfile=showProfile(state.writing);else if(state.activePageId==AppConstants::Convert)hasProfile=showProfile(state.converting);profile->setVisible(hasProfile);
        if(progress){ready->setStatus(QString::fromStdString(translations.text(operationKey)),QColor::fromRgb(running?L::RunningRgb:L::ReadingRgb));ready->setToolTip(progress->detailMessage?mediaText(translations,*progress->detailMessage):QString::fromStdString(progress->detail));return;}
        ready->setToolTip({});const auto active=state.converting&&state.converting->converting()?QString::fromLatin1(GwToolsConstants::Convert):state.greaseweazleTools?state.greaseweazleTools->activeOperation():QString{};
        const auto* key=(active==EngineConstants::Read||active==Infrastructure::MediaCommandConstants::ReadExplorer)?AppConstants::TabRead:active==EngineConstants::Write?AppConstants::TabWrite:active==GwToolsConstants::Convert?AppConstants::TabConvert:active.isEmpty()?ThemeConstants::StatusReadyShort:GwToolsConstants::RunningKey;ready->setStatus(QString::fromStdString(translations.text(key)),QColor::fromRgb(active.isEmpty()?L::IdleRgb:L::RunningRgb));
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
    if(state.explorer)openingSubscription_=state.explorer->subscribe(refreshOperation);
    if(state.visualization)visualizationSubscription_=state.visualization->subscribe(refreshOperation);
    if(state.reading)readingSubscription_=state.reading->subscribe(refreshOperation);
    if(state.writing)writingSubscription_=state.writing->subscribe(refreshOperation);
    if(state.explorerReading)explorerReadingSubscription_=state.explorerReading->subscribe(refreshOperation);
    auto* timer=new QTimer(this);timer->setInterval(LoadingConstants::TimerInterval);connect(timer,&QTimer::timeout,this,refreshOperation);timer->start();if(auto* tabs=window()->findChild<QTabWidget*>(ThemeConstants::WorkspaceTabs))connect(tabs,&QTabWidget::currentChanged,this,[refreshOperation]{refreshOperation();});
    refresh();
    languageSubscription_ = translations.subscribe(refresh);
    themeSubscription_ = themes.subscribe(refresh);
    hardwareSubscription_ = state.subscribeToHardware(refresh);
    if(state.greaseweazleTools)operationSubscription_=state.greaseweazleTools->subscribe(refreshOperation);
    if(state.converting)conversionSubscription_=state.converting->subscribe(refreshOperation);
}
}

