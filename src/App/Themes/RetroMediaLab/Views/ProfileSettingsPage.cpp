#include "ProfileSettingsPage.h"
#include "../ThemeConstants.h"
#include "Shared/Controls/Card.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
namespace RetroMediaLab {
ProfileSettingsPage::ProfileSettingsPage(ThemeService& themes, Translations::TranslationService& translations, QWidget* parent)
    : QWidget(parent), translations_(translations) {
    using namespace ThemeConstants::ProfilesPage;
    setObjectName(Page);
    auto* columns = new QHBoxLayout(this);
    columns->setContentsMargins(Padding, Padding, Padding, Padding);
    columns->setSpacing(Gap);
    for (std::size_t i = 0; i < lists_.size(); ++i) {
        auto* card = new Card(themes.data(), this);
        card->setMinimumWidth(0);
        auto* layout = new QVBoxLayout(card);
        layout->setContentsMargins(CardPadding, CardPadding, CardPadding, CardPadding);
        layout->setSpacing(Gap);
        titles_[i] = new QLabel(card);
        titles_[i]->setWordWrap(true);
        auto heading = titles_[i]->font();
        heading.setBold(true);
        titles_[i]->setFont(heading);
        lists_[i] = new QListWidget(card);
        lists_[i]->setObjectName(ListNames[i]);
        lists_[i]->setMinimumWidth(0);
        lists_[i]->setFrameShape(QFrame::NoFrame);
        lists_[i]->setSelectionMode(QAbstractItemView::SingleSelection);
        titles_[i]->setBuddy(lists_[i]);
        layout->addWidget(titles_[i]);
        layout->addWidget(lists_[i], 1);
        columns->addWidget(card, 1);
    }
    languageSubscription_ = translations.subscribe([this] { refreshTexts(); });
    refreshTexts();
}
void ProfileSettingsPage::refreshTexts() {
    for (std::size_t i = 0; i < lists_.size(); ++i) {
        const auto label = QString::fromStdString(translations_.text(ThemeConstants::ProfilesPage::LabelKeys[i]));
        titles_[i]->setText(label);
        lists_[i]->setAccessibleName(label);
    }
}
}
