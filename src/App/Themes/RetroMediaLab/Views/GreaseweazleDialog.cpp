#include "GreaseweazleDialog.h"
#include "GreaseweazleToolPage.h"
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include "../ThemeConstants.h"
#include <RetroMediaLab/Platform/WindowAppearance.h>
#include <QStackedWidget>
#include <QListWidget>
#include "Shared/Controls/Card.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QScreen>
#include <QTimer>
#include <QSignalBlocker>
#include <iterator>
namespace RetroMediaLab {
GreaseweazleDialog::GreaseweazleDialog(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QDialog(parent), themes_(themes), translations_(translations), state_(state) {
    using namespace ThemeConstants;
    namespace DialogConstants = ThemeConstants::GreaseweazleDialog;
    setObjectName(DialogConstants::ObjectName);
    setWindowModality(Qt::WindowModal);
    setWindowIcon(parent->windowIcon());
    setSizeGripEnabled(true);
    state.ensureMediaOperations(translations);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(PreferencesLayout::OuterPadding, PreferencesLayout::OuterPadding, PreferencesLayout::OuterPadding, PreferencesLayout::OuterPadding);
    auto* readerRow = new QHBoxLayout;
    readerLabel_ = new QLabel(this);
    readers_ = new QComboBox(this);
    readers_->setObjectName(DialogConstants::DeviceSelector);
    readerLabel_->setBuddy(readers_);
    readerRow->addWidget(readerLabel_);
    readerRow->addWidget(readers_, 1);
    layout->addLayout(readerRow);
    auto* body = new QHBoxLayout;
    body->setSpacing(DialogConstants::NavigationGap);
    auto* navigationCard = new Card(themes.data(), this);
    navigationCard->setFixedWidth(DialogConstants::NavigationWidth);
    auto* navigationLayout = new QVBoxLayout(navigationCard);
    navigationLayout->setContentsMargins(DialogConstants::NavigationPadding, DialogConstants::NavigationPadding, DialogConstants::NavigationPadding, DialogConstants::NavigationPadding);
    navigation_ = new QListWidget(navigationCard);
    navigation_->setObjectName(DialogConstants::Navigation);
    navigation_->setFrameShape(QFrame::NoFrame);
    navigation_->setWordWrap(true);
    navigation_->setTextElideMode(Qt::ElideNone);
    navigation_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationLayout->addWidget(navigation_);
    auto* contentCard = new Card(themes.data(), this);
    auto* contentLayout = new QVBoxLayout(contentCard);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    pages_ = new QStackedWidget(contentCard);
    pages_->setObjectName(DialogConstants::Pages);
    for (const auto& definition : gwToolDefinitions()) {
        auto* item = new QListWidgetItem(navigation_);
        item->setSizeHint(QSize(0, DialogConstants::NavigationRowHeight));
        auto* label = new QLabel(navigation_);
        label->setWordWrap(true);
        label->setTextFormat(Qt::PlainText);
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        label->setContentsMargins(DialogConstants::NavigationPadding, 0, DialogConstants::NavigationPadding, 0);
        navigation_->setItemWidget(item, label);
        pages_->addWidget(new GreaseweazleToolPage(definition, translations, state, pages_));
    }
    contentLayout->addWidget(pages_);
    body->addWidget(navigationCard);
    body->addWidget(contentCard, 1);
    layout->addLayout(body, 1);
    connect(navigation_, &QListWidget::currentRowChanged, this, [this](int index) {
        if (index < 0) return;
        pages_->setCurrentIndex(index);
        state_.greaseweazleTab = index;
    });
    navigation_->setCurrentRow(qBound(0, state.greaseweazleTab, pages_->count() - 1));
    close_ = new QPushButton(this);
    close_->setMinimumWidth(PreferencesLayout::ActionMinimumWidth);
    layout->addWidget(close_, 0, Qt::AlignRight);
    connect(close_, &QPushButton::clicked, this, [this] { reject(); });
    const auto available = parent->screen()->availableGeometry();
    const auto room = available.size() - QSize(PreferencesLayout::ScreenInset, PreferencesLayout::ScreenInset);
    setMinimumSize(qMin(PreferencesLayout::MinimumWidth, room.width()), qMin(PreferencesLayout::MinimumHeight, room.height()));
    resize(qMin(PreferencesLayout::DefaultWidth, room.width()), qMin(PreferencesLayout::MinimumWidth, room.height()));
    move(parent->geometry().center() - rect().center());
    if (!state.greaseweazleGeometry.isEmpty()) restoreGeometry(state.greaseweazleGeometry);
    connect(readers_, &QComboBox::currentIndexChanged, this, [this](int index) {
        state_.greaseweazleDeviceId = index >= 0 ? readers_->itemData(index).toString() : QString();
        for (int i = 0; i < pages_->count(); ++i) static_cast<GreaseweazleToolPage*>(pages_->widget(i))->refresh();
    });
    hardwareSubscription_ = state.subscribeToHardware([this] { refreshReaders(); });
    toolsSubscription_ = state.greaseweazleTools->subscribe([this] {
        const bool running = !state_.greaseweazleTools->activeOperation().isEmpty();
        readers_->setEnabled(!running && !closeRequested_);
        if (closeRequested_ && !running) QTimer::singleShot(0, this, [this] { done(QDialog::Rejected); });
    });
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    themeSubscription_ = themes.subscribe([this] { refreshAppearance(); });
    refreshTexts();
    refreshReaders();
    refreshAppearance();
}
void GreaseweazleDialog::refreshTexts() {
    setWindowTitle(QString::fromStdString(translations_.text(ThemeConstants::GreaseweazleDialog::Title)));
    const QSignalBlocker blocked(navigation_);
    for (int index = 0; index < pages_->count(); ++index) {
        const auto text = QString::fromStdString(translations_.text(ThemeConstants::GreaseweazleDialog::Labels[index]));
        auto* item = navigation_->item(index);
        item->setData(Qt::AccessibleTextRole, text);
        auto* label = static_cast<QLabel*>(navigation_->itemWidget(item));
        label->setText(text);
        label->setAccessibleName(text);
        label->setToolTip(text);
    }
    close_->setText(QString::fromStdString(translations_.text(ThemeConstants::PreferencesClose)));
    readerLabel_->setText(QString::fromStdString(translations_.text(ThemeConstants::GreaseweazleDialog::DeviceLabel)));
    refreshReaders();
}
void GreaseweazleDialog::refreshReaders() {
    const QSignalBlocker blocked(readers_);
    readers_->clear();
    for (const auto& device : state_.hardware().devices) {
        if (device.backend != HardwareConstants::Greaseweazle || device.category != HardwareConstants::Floppy) continue;
        auto label = QString::fromUtf8(ThemeConstants::GreaseweazleDialog::DeviceFormat).arg(device.name, device.address, device.selection);
        if (!device.available) label += ThemeConstants::HardwareDetailSeparator + QString::fromStdString(translations_.text(ThemeConstants::HardwareDisconnected));
        readers_->addItem(label, device.id);
    }
    if (state_.greaseweazleDeviceId.isEmpty() && readers_->count()) {
        const auto selected = readers_->findData(state_.selectedHardwareDeviceId);
        state_.greaseweazleDeviceId = readers_->itemData(selected >= 0 ? selected : 0).toString();
    }
    readers_->setCurrentIndex(readers_->findData(state_.greaseweazleDeviceId));
    readers_->setEnabled(state_.greaseweazleTools->activeOperation().isEmpty() && !closeRequested_);
    for (int i = 0; i < pages_->count(); ++i) static_cast<GreaseweazleToolPage*>(pages_->widget(i))->refresh();
}
void GreaseweazleDialog::refreshAppearance() {
    Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes_.data()->dark,
        themes_.data()->color(ThemeConstants::CaptionBrush).rgba(), themes_.data()->color(ThemeConstants::TextBrush).rgba(), themes_.data()->nativeSystemColors);
    update();
}
void GreaseweazleDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, [this] { refreshAppearance(); });
}
void GreaseweazleDialog::synchronizeState() {
    state_.greaseweazleTab = pages_->currentIndex();
    state_.greaseweazleGeometry = saveGeometry();
}
void GreaseweazleDialog::done(int result) {
    if (!state_.greaseweazleTools->activeOperation().isEmpty()) {
        closeRequested_ = true;
        pages_->setEnabled(false);
        navigation_->setEnabled(false);
        close_->setEnabled(false);
        readers_->setEnabled(false);
        state_.greaseweazleTools->stop();
        return;
    }
    closeRequested_ = false;
    pages_->setEnabled(true);
    navigation_->setEnabled(true);
    close_->setEnabled(true);
    refreshReaders();
    synchronizeState();
    state_.greaseweazleOpen = false;
    QDialog::done(result);
}
}
