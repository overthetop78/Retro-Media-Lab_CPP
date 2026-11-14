#include "../ThemeConstants.h"
#include "PreferencesDialog.h"
#include "ThemeSettingsPage.h"
#include "../ThemeIcons.h"
#include "Shared/Controls/Card.h"
#include <RetroMediaLab/Translations/LanguageCatalog.h>
#include <RetroMediaLab/Platform/WindowAppearance.h>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
namespace RetroMediaLab {
PreferencesDialog::PreferencesDialog(ThemeService& themes, Translations::TranslationService& translations,
    WorkspaceState& state, QWidget* parent)
    : QDialog(parent), themes_(themes), translations_(translations), state_(state) {
    setObjectName(ThemeConstants::PreferencesDialog);
    setWindowModality(Qt::WindowModal);
    setWindowIcon(parent->windowIcon());
    setSizeGripEnabled(true);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(ThemeConstants::PreferencesLayout::OuterPadding, ThemeConstants::PreferencesLayout::OuterPadding, ThemeConstants::PreferencesLayout::OuterPadding, ThemeConstants::PreferencesLayout::OuterPadding);
    sections_ = new QTabWidget(this);
    sections_->setObjectName(ThemeConstants::PreferencesSections);
    sections_->setIconSize(QSize(ThemeConstants::PreferencesLayout::TabIconExtent, ThemeConstants::PreferencesLayout::TabIconExtent));
    auto* general = new QWidget(sections_);
    auto* generalLayout = new QVBoxLayout(general);
    generalLayout->setContentsMargins(ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding);
    auto* row = new QHBoxLayout;
    row->setSpacing(ThemeConstants::PreferencesLayout::CardSpacing);
    auto* destinationCard = new Card(themes.data(), general);
    destinationCard->setObjectName(ThemeConstants::DestinationCard);
    auto* destinationLayout = new QVBoxLayout(destinationCard);
    destinationLayout->setContentsMargins(ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding);
    directoryLabel_ = new QLabel(destinationCard);
    directory_ = new QLineEdit(state.defaultImageDirectory(), destinationCard);
    directory_->setObjectName(ThemeConstants::DefaultImageDirectory);
    directory_->setMinimumWidth(ThemeConstants::PreferencesLayout::ActionMinimumWidth);
    directoryLabel_->setBuddy(directory_);
    browse_ = new QPushButton(destinationCard);
    browse_->setObjectName(ThemeConstants::BrowseImageDirectory);
    auto* directoryRow = new QHBoxLayout;
    directoryRow->addWidget(directory_, 1);
    directoryRow->addWidget(browse_);
    destinationLayout->addWidget(directoryLabel_);
    destinationLayout->addLayout(directoryRow);
    auto* languageCard = new Card(themes.data(), general);
    languageCard->setObjectName(ThemeConstants::LanguageCard);
    auto* languageLayout = new QVBoxLayout(languageCard);
    languageLayout->setContentsMargins(ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding, ThemeConstants::PreferencesLayout::CardPadding);
    languageLabel_ = new QLabel(languageCard);
    languages_ = new QComboBox(languageCard);
    languages_->setObjectName(ThemeConstants::PreferencesLanguage);
    for (const auto& language : Translations::availableLanguages())
        languages_->addItem(QString::fromStdString(language.nativeName), QString::fromStdString(language.code));
    languageLabel_->setBuddy(languages_);
    languageLayout->addWidget(languageLabel_);
    languageLayout->addWidget(languages_);
    row->addWidget(destinationCard, ThemeConstants::PreferencesLayout::DirectoryColumnWeight);
    row->addWidget(languageCard, ThemeConstants::PreferencesLayout::LanguageColumnWeight);
    generalLayout->addLayout(row);
    generalLayout->addStretch();
    sections_->addTab(general, QString());
    auto* themePage = new ThemeSettingsPage(themes, translations, parent, sections_);
    sections_->addTab(themePage, QString());
    layout->addWidget(sections_, 1);
    close_ = new QPushButton(this);
    close_->setObjectName(ThemeConstants::ClosePreferences);
    close_->setMinimumWidth(ThemeConstants::PreferencesLayout::ActionMinimumWidth);
    layout->addWidget(close_, 0, Qt::AlignRight);
    sections_->setCurrentIndex(state.preferencesSection == ThemeConstants::Themes ? 1 : 0);
    connect(sections_, &QTabWidget::currentChanged, this, [this](int index) {
        state_.preferencesSection = index == 1 ? ThemeConstants::Themes : ThemeConstants::General;
    });
    connect(languages_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) translations_.setLanguage(languages_->itemData(index).toString().toStdString());
    });
    connect(directory_, &QLineEdit::editingFinished, this, [this] {
        state_.setDefaultImageDirectory(directory_->text());
    });
    connect(browse_, &QPushButton::clicked, this, [this] { browseDirectory(); });
    connect(close_, &QPushButton::clicked, this, [this] { reject(); });
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    themeSubscription_ = themes.subscribe([this] { refreshAppearance(); });
    const auto room = screen()->availableGeometry().size() - QSize(ThemeConstants::PreferencesLayout::ScreenInset, ThemeConstants::PreferencesLayout::ScreenInset);
    setMinimumSize(qMin(ThemeConstants::PreferencesLayout::MinimumWidth, room.width()), qMin(ThemeConstants::PreferencesLayout::MinimumHeight, room.height()));
    resize(qMin(ThemeConstants::PreferencesLayout::DefaultWidth, room.width()), qMin(ThemeConstants::PreferencesLayout::MinimumWidth, room.height()));
    if (!state.preferencesGeometry.isEmpty()) restoreGeometry(state.preferencesGeometry);
    refreshTexts();
    refreshAppearance();
}
PreferencesDialog::~PreferencesDialog() = default;
void PreferencesDialog::browseDirectory() {
    if (!directoryPicker_) {
        directoryPicker_ = std::make_unique<QFileDialog>(this);
        directoryPicker_->setFileMode(QFileDialog::Directory);
        directoryPicker_->setOption(QFileDialog::ShowDirsOnly);
        directoryPicker_->setWindowModality(Qt::WindowModal);
        connect(directoryPicker_.get(), &QFileDialog::fileSelected, this, [this](const QString& path) {
            directory_->setText(path);
            state_.setDefaultImageDirectory(path);
        });
    }
    directoryPicker_->setWindowTitle(QString::fromStdString(translations_.text(ThemeConstants::PreferencesSelectDirectory)));
    directoryPicker_->setDirectory(directory_->text());
    directoryPicker_->open();
}
void PreferencesDialog::refreshTexts() {
    const auto text = [this](const char* key) { return QString::fromStdString(translations_.text(key)); };
    setWindowTitle(text(ThemeConstants::PreferencesTitle));
    sections_->setTabText(0, text(ThemeConstants::PreferencesGeneral));
    sections_->setTabText(1, text(ThemeConstants::PreferencesThemes));
    languageLabel_->setText(text(ThemeConstants::WindowLanguage));
    directoryLabel_->setText(text(ThemeConstants::PreferencesDefaultImageDirectory));
    directory_->setAccessibleName(directoryLabel_->text());
    browse_->setText(text(ThemeConstants::PreferencesBrowse));
    languages_->setAccessibleName(languageLabel_->text());
    const QSignalBlocker blocked(languages_);
    languages_->setCurrentIndex(languages_->findData(QString::fromStdString(translations_.language())));
    close_->setText(text(ThemeConstants::PreferencesClose));
}
void PreferencesDialog::refreshAppearance() {
    const auto icon = themeIcon(ThemeConstants::Preferences, themes_.data()->color(ThemeConstants::MutedTextBrush));
    sections_->setTabIcon(0, icon); sections_->setTabIcon(1, icon);
    Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes_.data()->dark,
        themes_.data()->color(ThemeConstants::CaptionBrush).rgba(), themes_.data()->color(ThemeConstants::TextBrush).rgba(),
        themes_.data()->nativeSystemColors);
    update();
}
void PreferencesDialog::synchronizeState() {
    state_.setDefaultImageDirectory(directory_->text());
    state_.preferencesGeometry = saveGeometry();
}
void PreferencesDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, [this] { refreshAppearance(); });
}
void PreferencesDialog::done(int result) {
    synchronizeState();
    state_.preferencesOpen = false;
    QDialog::done(result);
}
}
