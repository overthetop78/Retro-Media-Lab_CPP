#include "ConversionSettingsPage.h"
#include "../ThemeConstants.h"
#include "../ThemeIcons.h"
#include "Shared/Controls/Card.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>
namespace RetroMediaLab {
using namespace ConversionConstants;
ConversionSettingsPage::ConversionSettingsPage(ThemeService& themes,
    Translations::TranslationService& translations, WorkspaceState& state, QWidget* parent)
    : QWidget(parent), translations_(translations), state_(state) {
    setObjectName(Page);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content = new QWidget(scroll);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(Padding, Padding, Padding, Padding);
    layout->setSpacing(Gap);
    auto* columns = new QHBoxLayout;
    columns->setSpacing(Gap);
    const auto card = [&](QWidget* owner) {
        auto* result = new Card(themes.data(), owner);
        auto* body = new QVBoxLayout(result);
        body->setContentsMargins(Padding, Padding, Padding, Padding);
        body->setSpacing(Gap);
        return std::pair{result, body};
    };
    const auto label = [&](const char* key, QVBoxLayout* body) {
        auto* result = new QLabel(content);
        result->setWordWrap(true);
        labels_.push_back({result, key});
        body->addWidget(result);
        return result;
    };
    auto [settingsCard, settingsLayout] = card(content);
    enabled_ = new QCheckBox(settingsCard);
    enabled_->setObjectName(EnabledInput);
    enabled_->setChecked(state.conversion().addTags);
    settingsLayout->addWidget(enabled_);
    auto* presetLabel = label(PresetLabel, settingsLayout);
    presets_ = new QComboBox(settingsCard);
    presets_->setObjectName(PresetInput);
    presetLabel->setBuddy(presets_);
    settingsLayout->addWidget(presets_);
    auto* patternLabel = label(PatternLabel, settingsLayout);
    pattern_ = new QLineEdit(state.conversion().tagPattern, settingsCard);
    pattern_->setObjectName(PatternInput);
    patternLabel->setBuddy(pattern_);
    pattern_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    settingsLayout->addWidget(pattern_);
    auto* historyLabel = label(HistoryLabel, settingsLayout);
    recent_ = new QListWidget(settingsCard);
    recent_->setObjectName(RecentInput);
    recent_->setFixedHeight(HistoryHeight);
    recent_->setTextElideMode(Qt::ElideRight);
    historyLabel->setBuddy(recent_);
    settingsLayout->addWidget(recent_);
    settingsLayout->addStretch();
    auto [variablesCard, variablesLayout] = card(content);
    label(VariablesLabel, variablesLayout);
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(Gap);
    grid->setVerticalSpacing(VariableRowGap);
    int row = 0;
    for (const auto& variable : conversionVariables()) {
        auto* token = new QPushButton(QString::fromLatin1(variable.value), variablesCard);
        token->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        token->setMinimumWidth(TokenWidth);
        token->setAutoDefault(false);
        auto* description = new QLabel(variablesCard);
        description->setWordWrap(true);
        token->setFixedHeight(qMax(token->fontMetrics().height(), description->fontMetrics().height()) + TokenVerticalPadding);
        labels_.push_back({description, variable.label});
        variables_.push_back({token, variable.label});
        grid->addWidget(token, row, 0);
        grid->addWidget(description, row, 1);
        connect(token, &QPushButton::clicked, this, [this, value = variable.value] {
            pattern_->insert(QString::fromLatin1(value));
            pattern_->setFocus();
        });
        ++row;
    }
    grid->setColumnStretch(1, 1);
    variablesLayout->addLayout(grid);
    variablesLayout->addStretch();
    columns->addWidget(settingsCard, 1);
    columns->addWidget(variablesCard, 1);
    layout->addLayout(columns, 1);
    auto [previewCard, previewLayout] = card(content);
    auto* previewRow = new QHBoxLayout;
    preview_ = new QLabel(previewCard);
    preview_->setObjectName(Preview);
    preview_->setWordWrap(true);
    preview_->setTextFormat(Qt::PlainText);
    preview_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    preview_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    next_ = new QPushButton(previewCard);
    next_->setObjectName(NextExample);
    next_->setAutoDefault(false);
    previewRow->addWidget(preview_, 1);
    previewRow->addWidget(next_);
    previewLayout->addLayout(previewRow);
    layout->addWidget(previewCard);
    scroll->setWidget(content);
    outer->addWidget(scroll);
    connect(enabled_, &QCheckBox::toggled, this, [this](bool checked) {
        auto settings = state_.conversion();
        settings.addTags = checked;
        state_.setConversion(std::move(settings));
    });
    connect(pattern_, &QLineEdit::textChanged, this, [this] { selectPreset(); refreshPreview(); });
    connect(pattern_, &QLineEdit::editingFinished, this, [this] { commitEdits(); });
    connect(presets_, &QComboBox::activated, this, [this](int index) {
        if (index < 0) return;
        pattern_->setText(presets_->itemData(index).toString());
        commitEdits();
    });
    connect(recent_, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        if (!(item->flags() & Qt::ItemIsEnabled)) return;
        pattern_->setText(item->data(Qt::UserRole).toString());
        commitEdits();
    });
    connect(next_, &QPushButton::clicked, this, [this] {
        exampleIndex_ = (exampleIndex_ + 1) % ExampleCount;
        refreshPreview();
    });
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    refreshTexts();
    refreshHistory();
}
void ConversionSettingsPage::commitEdits() {
    auto settings = state_.conversion();
    settings.tagPattern = pattern_->text();
    settings.rememberCustomPattern();
    state_.setConversion(std::move(settings));
    refreshHistory();
}
void ConversionSettingsPage::selectPreset() {
    const QSignalBlocker blocked(presets_);
    int selected = NoPreset;
    for (int index = 0; index < presets_->count(); ++index)
        if (presets_->itemData(index).toString().compare(pattern_->text(), Qt::CaseInsensitive) == 0) selected = index;
    presets_->setCurrentIndex(selected);
}
void ConversionSettingsPage::refreshHistory() {
    recent_->clear();
    for (const auto& pattern : state_.conversion().recentCustomTagPatterns) {
        auto* item = new QListWidgetItem(pattern, recent_);
        item->setData(Qt::UserRole, pattern);
        item->setToolTip(pattern);
    }
    if (recent_->count() == 0) {
        auto* item = new QListWidgetItem(QString::fromUtf8(EmptyHistory), recent_);
        item->setFlags(Qt::NoItemFlags);
    }
}
void ConversionSettingsPage::refreshPreview() {
    auto text = QString::fromStdString(translations_.text(PreviewLabel));
    preview_->setText(text.replace(PreviewPlaceholder, conversionExample(pattern_->text(), exampleIndex_)));
}
void ConversionSettingsPage::refreshTexts() {
    const auto text = [this](const char* key) { return QString::fromStdString(translations_.text(key)); };
    enabled_->setText(text(UseTags));
    for (const auto& [label, key] : labels_) label->setText(text(key));
    for (const auto& [button, key] : variables_) button->setToolTip(text(key));
    const QSignalBlocker blocked(presets_);
    presets_->clear();
    for (const auto& preset : conversionPresets()) presets_->addItem(text(preset.label), QString::fromLatin1(preset.value));
    selectPreset();
    presets_->setAccessibleName(text(PresetLabel));
    pattern_->setAccessibleName(text(PatternLabel));
    recent_->setAccessibleName(text(HistoryLabel));
    next_->setText(text(NextLabel));
    refreshPreview();
}
}
