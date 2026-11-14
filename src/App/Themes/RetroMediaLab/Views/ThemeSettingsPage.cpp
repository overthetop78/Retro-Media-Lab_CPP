#include "../ThemeConstants.h"
#include "ThemeSettingsPage.h"
#include "ThemePreview.h"
#include <QPushButton>
#include "Shared/Controls/Card.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <utility>
namespace RetroMediaLab {
ThemeSettingsPage::ThemeSettingsPage(ThemeService& themes, Translations::TranslationService& translations,
    const WorkspaceState& state, QWidget* previewSource, QWidget* parent)
    : QWidget(parent), themes_(themes), translations_(translations),
      pendingAppearance_(themes.settings().value(ThemeConstants::Appearance).toString(ThemeConstants::Light)) {
    setObjectName(ThemeConstants::ThemeSettingsPage);
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(ThemeConstants::ThemeSettings::PagePadding, ThemeConstants::ThemeSettings::PagePadding, ThemeConstants::ThemeSettings::PagePadding, ThemeConstants::ThemeSettings::PagePadding);
    layout->setSpacing(ThemeConstants::ThemeSettings::ColumnSpacing);
    auto* listCard = new Card(themes.data(), this);
    listCard->setObjectName(ThemeConstants::AvailableThemesCard);
    listCard->setFixedWidth(ThemeConstants::ThemeSettings::ListWidth);
    auto* left = new QVBoxLayout(listCard);
    left->setContentsMargins(ThemeConstants::ThemeSettings::DetailSpacing, ThemeConstants::ThemeSettings::PagePadding, ThemeConstants::ThemeSettings::DetailSpacing, ThemeConstants::ThemeSettings::DetailSpacing);
    availableTitle_ = new QLabel(listCard);
    auto bold = availableTitle_->font();
    bold.setWeight(QFont::DemiBold);
    availableTitle_->setFont(bold);
    availableTitle_->setWordWrap(true);
    left->addWidget(availableTitle_);
    available_ = new QListWidget(listCard);
    available_->setObjectName(ThemeConstants::AvailableThemes);
    available_->setFrameShape(QFrame::NoFrame);
    auto* item = new QListWidgetItem(available_);
    item->setData(Qt::UserRole, ThemeConstants::RetroMediaLab);
    item->setSizeHint(QSize(0, ThemeConstants::ThemeSettings::ListEntryHeight));
    auto* entry = new QWidget(available_);
    auto* entryLayout = new QVBoxLayout(entry);
    entryLayout->setContentsMargins(ThemeConstants::ThemeSettings::ListEntryHorizontalInset, ThemeConstants::ThemeSettings::ListEntryVerticalInset, ThemeConstants::ThemeSettings::ListEntryHorizontalInset, ThemeConstants::ThemeSettings::ListEntryVerticalInset);
    listName_ = new QLabel(QString::fromStdString(translations.text(ThemeConstants::ThemeName)), entry);
    listName_->setFont(bold);
    variants_ = new QLabel(entry);
    variants_->setWordWrap(true);
    entryLayout->addWidget(listName_);
    entryLayout->addWidget(variants_);
    available_->setItemWidget(item, entry);
    available_->setCurrentRow(0);
    left->addWidget(available_, 1);
    layout->addWidget(listCard);
    auto* right = new QVBoxLayout;
    right->setSpacing(ThemeConstants::ThemeSettings::DetailSpacing);
    preview_ = new ThemePreview(themes, translations, state, previewSource, this);
    right->addWidget(preview_, 1);
    name_ = new QLabel(QString::fromStdString(translations.text(ThemeConstants::ThemeName)), this);
    auto heading = name_->font();
    heading.setPixelSize(ThemeConstants::ThemeSettings::HeadingFontSize);
    heading.setWeight(QFont::DemiBold);
    name_->setFont(heading);
    right->addWidget(name_);
    description_ = new QLabel(this);
    description_->setWordWrap(true);
    right->addWidget(description_);
    appearance_ = new QLabel(this);
    right->addWidget(appearance_);
    system_ = new QRadioButton(this);
    system_->setObjectName(ThemeConstants::AppearanceSystem);
    light_ = new QRadioButton(this);
    light_->setObjectName(ThemeConstants::AppearanceLight);
    dark_ = new QRadioButton(this);
    dark_->setObjectName(ThemeConstants::AppearanceDark);
    auto* variants = new QHBoxLayout;
    variants->addWidget(system_);
    variants->addWidget(light_);
    variants->addWidget(dark_);
    variants->addStretch();
    right->addLayout(variants);
    active_ = new QLabel(this);
    active_->setObjectName(ThemeConstants::ActiveTheme);
    active_->setWordWrap(true);
    right->addWidget(active_);
    persistence_ = new QLabel(this);
    persistence_->setWordWrap(true);
    right->addWidget(persistence_);
    right->addStretch();
    choose_ = new QPushButton(this);
    choose_->setObjectName(ThemeConstants::ChooseTheme);
    choose_->setMinimumWidth(ThemeConstants::PreferencesLayout::ActionMinimumWidth);
    right->addWidget(choose_, 0, Qt::AlignRight);
    layout->addLayout(right, 1);
    connect(choose_, &QPushButton::clicked, this, [this] {
        themes_.applySettings({{ThemeConstants::Appearance, pendingAppearance_}});
    });
    connect(available_, &QListWidget::currentRowChanged, this, [this] { refreshSelection(); });
    for (const auto& option : {std::pair{system_, ThemeConstants::System},
                              std::pair{light_, ThemeConstants::Light}, std::pair{dark_, ThemeConstants::Dark}}) {
        connect(option.first, &QRadioButton::toggled, this, [this, variant = option.second](bool checked) {
            if (checked) {
                pendingAppearance_ = variant;
                refreshSelection();
            }
        });
    }
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    themeSubscription_ = themes.subscribe([this] { refreshAppearance(); });
    refreshTexts();
    refreshAppearance();
}
void ThemeSettingsPage::refreshTexts() {
    const auto text = [this](const char* key) { return QString::fromStdString(translations_.text(key)); };
    availableTitle_->setText(text(ThemeConstants::ThemesAvailable));
    available_->setAccessibleName(availableTitle_->text());
    system_->setText(text(ThemeConstants::ThemesSystem));
    light_->setText(text(ThemeConstants::WindowLight));
    dark_->setText(text(ThemeConstants::WindowDark));
    variants_->setText(text(ThemeConstants::ThemesVariants).replace(ThemeConstants::System2, system_->text())
        .replace(ThemeConstants::Light2, light_->text()).replace(ThemeConstants::Dark2, dark_->text()));
    available_->item(0)->setData(Qt::AccessibleTextRole, listName_->text() + ThemeConstants::Space + variants_->text());
    description_->setText(text(ThemeConstants::ThemesDescription));
    appearance_->setText(text(ThemeConstants::WindowTheme));
    choose_->setText(text(ThemeConstants::ThemesChoose));
    persistence_->setText(text(ThemeConstants::ThemesPersisted).replace(ThemeConstants::ChooseParameter, choose_->text()));
    preview_->setAccessibleName(text(ThemeConstants::ThemesPreview));
    const auto appearance = themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::System ? system_->text()
        : themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::Dark ? dark_->text() : light_->text();
    active_->setText(text(ThemeConstants::ThemesActive).replace(ThemeConstants::Theme, name_->text()).replace(ThemeConstants::Appearance2, appearance));
    refreshSelection();
}
void ThemeSettingsPage::refreshAppearance() {
    const QSignalBlocker systemBlocked(system_), lightBlocked(light_), darkBlocked(dark_);
    system_->setChecked(pendingAppearance_ == ThemeConstants::System);
    light_->setChecked(pendingAppearance_ == ThemeConstants::Light);
    dark_->setChecked(pendingAppearance_ == ThemeConstants::Dark);
    auto colors = available_->palette();
    colors.setColor(QPalette::Base, themes_.data()->color(ThemeConstants::CardBrush));
    available_->setPalette(colors);
    refreshTexts();
}
void ThemeSettingsPage::refreshSelection() {
    choose_->setEnabled(pendingAppearance_ != themes_.settings().value(ThemeConstants::Appearance).toString());
    preview_->setSettings({{ThemeConstants::Appearance, pendingAppearance_}});
}

}
