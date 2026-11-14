#include "../ThemeConstants.h"
#include "ThemeSettingsPage.h"
#include "Shared/Controls/Card.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>
namespace RetroMediaLab {
class ThemePreview final : public QWidget {
public:
    ThemePreview(std::shared_ptr<const ThemeData> data, QWidget* source, QWidget* parent)
        : QWidget(parent), data_(std::move(data)), source_(source) {
        setObjectName(ThemeConstants::ThemePreview);
        setMinimumHeight(ThemeConstants::ThemeSettings::PreviewMinimumHeight);
        setMaximumHeight(ThemeConstants::ThemeSettings::PreviewMaximumHeight);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void refresh() {
        if (scheduled_) return;
        scheduled_ = true;
        QTimer::singleShot(0, this, [this] {
            scheduled_ = false;
            if (source_) image_ = source_->grab();
            update();
        });
    }
protected:
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        refresh();
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(data_->color(ThemeConstants::ControlBrush));
        painter.setPen(data_->color(ThemeConstants::BorderBrush));
        painter.drawRoundedRect(QRectF(rect()).adjusted(ThemeConstants::ThemeSettings::BorderInset, ThemeConstants::ThemeSettings::BorderInset, -ThemeConstants::ThemeSettings::BorderInset, -ThemeConstants::ThemeSettings::BorderInset), data_->cardRadius, data_->cardRadius);
        if (image_.isNull()) return;
        const auto room = size() - QSize(ThemeConstants::ThemeSettings::PreviewInset, ThemeConstants::ThemeSettings::PreviewInset);
        auto scaled = image_.size();
        scaled.scale(room, Qt::KeepAspectRatio);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(QRect(QPoint((width() - scaled.width()) / ThemeConstants::ThemeSettings::CenterDivisor, (height() - scaled.height()) / ThemeConstants::ThemeSettings::CenterDivisor), scaled), image_);
    }
private:
    std::shared_ptr<const ThemeData> data_;
    QPointer<QWidget> source_;
    QPixmap image_;
    bool scheduled_ = false;
};
ThemeSettingsPage::ThemeSettingsPage(ThemeService& themes, Translations::TranslationService& translations,
    QWidget* previewSource, QWidget* parent) : QWidget(parent), themes_(themes), translations_(translations) {
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
    preview_ = new ThemePreview(themes.data(), previewSource, this);
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
    choose_->setMinimumWidth(ThemeConstants::ThemeSettings::ActionMinimumWidth);
    choose_->setEnabled(false); // The only available theme is already active.
    right->addWidget(choose_, 0, Qt::AlignRight);
    layout->addLayout(right, 1);
    for (const auto& option : {std::pair{system_, ThemeConstants::System},
                              std::pair{light_, ThemeConstants::Light}, std::pair{dark_, ThemeConstants::Dark}}) {
        connect(option.first, &QRadioButton::toggled, this, [this, variant = option.second](bool checked) {
            if (checked) themes_.applySettings({{ThemeConstants::Appearance, variant}});
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
    persistence_->setText(text(ThemeConstants::ThemesPersisted));
    choose_->setText(text(ThemeConstants::ThemesChoose));
    preview_->setAccessibleName(text(ThemeConstants::ThemesPreview));
    const auto appearance = themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::System ? system_->text()
        : themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::Dark ? dark_->text() : light_->text();
    active_->setText(text(ThemeConstants::ThemesActive).replace(ThemeConstants::Theme, name_->text()).replace(ThemeConstants::Appearance2, appearance));
    preview_->refresh();
}
void ThemeSettingsPage::refreshAppearance() {
    const QSignalBlocker systemBlocked(system_), lightBlocked(light_), darkBlocked(dark_);
    system_->setChecked(themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::System);
    light_->setChecked(themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::Light);
    dark_->setChecked(themes_.settings().value(ThemeConstants::Appearance) == ThemeConstants::Dark);
    auto colors = available_->palette();
    colors.setColor(QPalette::Base, themes_.data()->color(ThemeConstants::CardBrush));
    available_->setPalette(colors);
    refreshTexts();
}
}
