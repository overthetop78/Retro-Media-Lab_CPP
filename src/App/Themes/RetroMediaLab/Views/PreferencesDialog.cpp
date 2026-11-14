#include "PreferencesDialog.h"
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
    setObjectName("preferencesDialog");
    setWindowModality(Qt::WindowModal);
    setWindowIcon(parent->windowIcon());
    setSizeGripEnabled(true);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    sections_ = new QTabWidget(this);
    sections_->setObjectName("preferencesSections");
    sections_->setIconSize(QSize(17, 17));
    auto* general = new QWidget(sections_);
    auto* generalLayout = new QVBoxLayout(general);
    generalLayout->setContentsMargins(18, 18, 18, 18);
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* destinationCard = new Card(themes.data(), general);
    destinationCard->setObjectName("destinationCard");
    auto* destinationLayout = new QVBoxLayout(destinationCard);
    destinationLayout->setContentsMargins(18, 18, 18, 18);
    directoryLabel_ = new QLabel(destinationCard);
    directory_ = new QLineEdit(state.defaultImageDirectory(), destinationCard);
    directory_->setObjectName("defaultImageDirectory");
    directory_->setMinimumWidth(120);
    directoryLabel_->setBuddy(directory_);
    browse_ = new QPushButton(destinationCard);
    browse_->setObjectName("browseImageDirectory");
    auto* directoryRow = new QHBoxLayout;
    directoryRow->addWidget(directory_, 1);
    directoryRow->addWidget(browse_);
    destinationLayout->addWidget(directoryLabel_);
    destinationLayout->addLayout(directoryRow);
    auto* languageCard = new Card(themes.data(), general);
    languageCard->setObjectName("languageCard");
    auto* languageLayout = new QVBoxLayout(languageCard);
    languageLayout->setContentsMargins(18, 18, 18, 18);
    languageLabel_ = new QLabel(languageCard);
    languages_ = new QComboBox(languageCard);
    languages_->setObjectName("preferencesLanguage");
    for (const auto& language : Translations::availableLanguages())
        languages_->addItem(QString::fromStdString(language.nativeName), QString::fromStdString(language.code));
    languageLabel_->setBuddy(languages_);
    languageLayout->addWidget(languageLabel_);
    languageLayout->addWidget(languages_);
    row->addWidget(destinationCard, 3);
    row->addWidget(languageCard, 2);
    generalLayout->addLayout(row);
    generalLayout->addStretch();
    sections_->addTab(general, QString());
    auto* themePage = new QWidget(sections_);
    auto* themeLayout = new QVBoxLayout(themePage);
    themeLayout->setContentsMargins(18, 18, 18, 18);
    auto* themeCard = new Card(themes.data(), themePage);
    auto* options = new QVBoxLayout(themeCard);
    options->setContentsMargins(18, 18, 18, 18);
    themeName_ = new QLabel(QString::fromStdString(translations.text("Theme.Name")), themeCard);
    auto heading = themeName_->font();
    heading.setWeight(QFont::DemiBold);
    themeName_->setFont(heading);
    description_ = new QLabel(themeCard);
    description_->setWordWrap(true);
    appearanceLabel_ = new QLabel(themeCard);
    light_ = new QRadioButton(themeCard);
    light_->setObjectName("appearanceLight");
    dark_ = new QRadioButton(themeCard);
    dark_->setObjectName("appearanceDark");
    auto* variants = new QHBoxLayout;
    variants->addWidget(light_); variants->addWidget(dark_); variants->addStretch();
    options->addWidget(themeName_);
    options->addWidget(description_);
    options->addSpacing(16);
    options->addWidget(appearanceLabel_);
    options->addLayout(variants);
    themeLayout->addWidget(themeCard);
    themeLayout->addStretch();
    sections_->addTab(themePage, QString());
    layout->addWidget(sections_, 1);
    close_ = new QPushButton(this);
    close_->setObjectName("closePreferences");
    close_->setMinimumWidth(120);
    layout->addWidget(close_, 0, Qt::AlignRight);
    sections_->setCurrentIndex(state.preferencesSection == "themes" ? 1 : 0);
    connect(sections_, &QTabWidget::currentChanged, this, [this](int index) {
        state_.preferencesSection = index == 1 ? "themes" : "general";
    });
    connect(languages_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) translations_.setLanguage(languages_->itemData(index).toString().toStdString());
    });
    connect(directory_, &QLineEdit::editingFinished, this, [this] {
        state_.setDefaultImageDirectory(directory_->text());
    });
    connect(browse_, &QPushButton::clicked, this, [this] { browseDirectory(); });
    connect(light_, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) themes_.setVariant(ThemeVariant::Light);
    });
    connect(dark_, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) themes_.setVariant(ThemeVariant::Dark);
    });
    connect(close_, &QPushButton::clicked, this, [this] { reject(); });
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    themeSubscription_ = themes.subscribe([this] { refreshAppearance(); });
    const auto room = screen()->availableGeometry().size() - QSize(48, 48);
    setMinimumSize(qMin(640, room.width()), qMin(400, room.height()));
    resize(qMin(1100, room.width()), qMin(640, room.height()));
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
    directoryPicker_->setWindowTitle(QString::fromStdString(translations_.text("Preferences.SelectDirectory")));
    directoryPicker_->setDirectory(directory_->text());
    directoryPicker_->open();
}
void PreferencesDialog::refreshTexts() {
    const auto text = [this](const char* key) { return QString::fromStdString(translations_.text(key)); };
    setWindowTitle(text("Preferences.Title"));
    sections_->setTabText(0, text("Preferences.General"));
    sections_->setTabText(1, text("Preferences.Themes"));
    languageLabel_->setText(text("Window.Language"));
    directoryLabel_->setText(text("Preferences.DefaultImageDirectory"));
    directory_->setAccessibleName(directoryLabel_->text());
    browse_->setText(text("Preferences.Browse"));
    languages_->setAccessibleName(languageLabel_->text());
    const QSignalBlocker blocked(languages_);
    languages_->setCurrentIndex(languages_->findData(QString::fromStdString(translations_.language())));
    appearanceLabel_->setText(text("Window.Theme"));
    light_->setText(text("Window.Light"));
    dark_->setText(text("Window.Dark"));
    description_->setText(text("Preferences.ThemeDescription"));
    close_->setText(text("Preferences.Close"));
}
void PreferencesDialog::refreshAppearance() {
    const QSignalBlocker lightBlocked(light_), darkBlocked(dark_);
    light_->setChecked(themes_.variant() == ThemeVariant::Light);
    dark_->setChecked(themes_.variant() == ThemeVariant::Dark);
    const auto icon = themeIcon("preferences", themes_.data()->color("MutedTextBrush"));
    sections_->setTabIcon(0, icon); sections_->setTabIcon(1, icon);
    Platform::applyWindowAppearance(static_cast<std::uintptr_t>(winId()), themes_.variant() == ThemeVariant::Dark,
        themes_.data()->color("CaptionBrush").rgba(), themes_.data()->color("TextBrush").rgba());
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
