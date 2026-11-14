#include "RetroMediaLabLayout.h"
#include "Shared/Controls/Card.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
namespace RetroMediaLab {
RetroMediaLabLayout::RetroMediaLabLayout(ThemeService& themes, Translations::TranslationService& translations, QWidget* parent)
    : QWidget(parent), themes_(themes), translations_(translations) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 20, 24, 16);
    outer->setSpacing(20);
    auto* header = new QHBoxLayout;
    outer->addLayout(header);
    auto* brand = new QLabel(QString::fromStdString(translations.text("App.Title")), this);
    auto brandFont = brand->font();
    brandFont.setPixelSize(22);
    brandFont.setWeight(QFont::DemiBold);
    brand->setFont(brandFont);
    header->addWidget(brand);
    header->addStretch();
    themeLabel_ = new QLabel(this);
    variants_ = new QComboBox(this);
    variants_->setObjectName("themeVariant");
    variants_->addItem(QString(), static_cast<int>(ThemeVariant::Light));
    variants_->addItem(QString(), static_cast<int>(ThemeVariant::Dark));
    variants_->setMinimumWidth(125);
    variants_->setCurrentIndex(themes.variant() == ThemeVariant::Dark ? 1 : 0);
    themeLabel_->setBuddy(variants_);
    header->addWidget(themeLabel_);
    header->addWidget(variants_);
    header->addSpacing(14);
    languageLabel_ = new QLabel(this);
    languages_ = new QComboBox(this);
    languages_->setObjectName("language");
    // Native language names are invariants supplied by the catalog.
    languages_->addItem(QString::fromStdString(translations.text("Language.en-US")), "en-US");
    languages_->addItem(QString::fromStdString(translations.text("Language.fr-FR")), "fr-FR");
    languages_->setMinimumWidth(140);
    languageLabel_->setBuddy(languages_);
    header->addWidget(languageLabel_);
    header->addWidget(languages_);
    auto* card = new Card(themes.data(), this);
    card->setObjectName("workspace");
    auto* cardLayout = new QVBoxLayout(card);
    const auto padding = themes.data()->cardPadding;
    cardLayout->setContentsMargins(padding, padding, padding, padding);
    cardLayout->addStretch();
    welcome_ = new QLabel(card);
    auto headingFont = welcome_->font();
    headingFont.setPixelSize(28);
    headingFont.setWeight(QFont::DemiBold);
    welcome_->setFont(headingFont);
    welcome_->setAlignment(Qt::AlignCenter);
    welcome_->setWordWrap(true);
    description_ = new QLabel(card);
    description_->setWordWrap(true);
    description_->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(welcome_);
    cardLayout->addSpacing(8);
    cardLayout->addWidget(description_);
    cardLayout->addStretch();
    outer->addWidget(card, 1);
    ready_ = new QLabel(this);
    outer->addWidget(ready_);
    connect(variants_, &QComboBox::currentIndexChanged, this, [this](int index) {
        themes_.setVariant(static_cast<ThemeVariant>(variants_->itemData(index).toInt()));
    });
    connect(languages_, &QComboBox::currentIndexChanged, this, [this](int index) {
        translations_.setLanguage(languages_->itemData(index).toString().toStdString());
    });
    languageSubscription_ = translations_.subscribe([this] { refreshTexts(); });
    themeSubscription_ = themes_.subscribe([this] { refreshColors(); });
    refreshTexts();
    refreshColors();
}
void RetroMediaLabLayout::refreshTexts() {
    const QSignalBlocker variantsBlocked(variants_);
    const QSignalBlocker languagesBlocked(languages_);
    const auto text = [this](const char* key) { return QString::fromStdString(translations_.text(key)); };
    themeLabel_->setText(text("Window.Theme"));
    variants_->setItemText(0, text("Window.Light"));
    variants_->setItemText(1, text("Window.Dark"));
    languageLabel_->setText(text("Window.Language"));
    languages_->setCurrentIndex(languages_->findData(QString::fromStdString(translations_.language())));
    welcome_->setText(text("Window.Welcome"));
    description_->setText(text("Window.Description"));
    ready_->setText(text("Window.Ready"));
    variants_->setAccessibleName(themeLabel_->text());
    languages_->setAccessibleName(languageLabel_->text());
}
void RetroMediaLabLayout::refreshColors() {
    const QSignalBlocker blocked(variants_);
    variants_->setCurrentIndex(themes_.variant() == ThemeVariant::Dark ? 1 : 0);
    auto muted = palette();
    muted.setColor(QPalette::WindowText, themes_.data()->color("MutedTextBrush"));
    for (auto* label : {description_, ready_, themeLabel_, languageLabel_}) label->setPalette(muted);
    update();
}
}
