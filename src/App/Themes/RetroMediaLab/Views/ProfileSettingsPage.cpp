#include "ProfileSettingsPage.h"
#include "../ThemeConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include "Profiles/ProfileConstants.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QAbstractButton>
#include <stdexcept>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenu>
#include <QSignalBlocker>
namespace RetroMediaLab {
ProfileSettingsPage::ProfileSettingsPage(ThemeService& themes, Translations::TranslationService& translations, WorkspaceState& state,QWidget* parent)
    : QWidget(parent), translations_(translations),state_(state) {
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
        auto* actions=new QHBoxLayout;rename_[i]=new QPushButton(card);remove_[i]=new QPushButton(card);actions->addWidget(rename_[i]);actions->addWidget(remove_[i]);layout->addLayout(actions);
        lists_[i]->installEventFilter(this);lists_[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(rename_[i],&QPushButton::clicked,this,[this,i]{rename(i);});connect(remove_[i],&QPushButton::clicked,this,[this,i]{remove(i);});
        connect(lists_[i],&QListWidget::itemDoubleClicked,this,[this,i]{rename(i);});
        connect(lists_[i],&QListWidget::currentRowChanged,this,[this,i](int row){const bool editable=row>0;rename_[i]->setEnabled(editable);remove_[i]->setEnabled(editable);});
        connect(lists_[i],&QListWidget::customContextMenuRequested,this,[this,i](const QPoint& position){if(auto* item=lists_[i]->itemAt(position))lists_[i]->setCurrentItem(item);if(lists_[i]->currentRow()<=0)return;QMenu menu(this);auto* renameAction=menu.addAction(rename_[i]->text());auto* deleteAction=menu.addAction(remove_[i]->text());const auto* selected=menu.exec(lists_[i]->viewport()->mapToGlobal(position));if(selected==renameAction)rename(i);else if(selected==deleteAction)remove(i);});
        columns->addWidget(card, 1);
    }
    languageSubscription_ = translations.subscribe([this] { refreshTexts();refresh(); });
    profileSubscription_=state.profiles->subscribe([this]{refresh();});
    refreshTexts();
    refresh();
}
void ProfileSettingsPage::refreshTexts() {
    for (std::size_t i = 0; i < lists_.size(); ++i) {
        const auto label = QString::fromStdString(translations_.text(ThemeConstants::ProfilesPage::LabelKeys[i]));
        titles_[i]->setText(label);
        lists_[i]->setAccessibleName(label);
        rename_[i]->setText(QString::fromStdString(translations_.text(Infrastructure::ProfileConstants::Rename)));remove_[i]->setText(QString::fromStdString(translations_.text(Infrastructure::ProfileConstants::DeleteAction)));
    }
}
void ProfileSettingsPage::refresh(){
    namespace P=Infrastructure::ProfileConstants;
    for(std::size_t i=0;i<lists_.size();++i){
        auto* list=lists_[i];const auto selected=list->currentItem()?list->currentItem()->data(Qt::UserRole).toString():QString{};const QSignalBlocker blocked(list);list->clear();
        for(const auto& profile:state_.profiles->forOperation(static_cast<Infrastructure::ProfileOperation>(i))){auto* item=new QListWidgetItem(profile.system?QString::fromStdString(translations_.text(P::DefaultLabel)):profile.name,list);item->setData(Qt::UserRole,profile.id);if(profile.id==selected)list->setCurrentItem(item);}
        if(list->currentRow()<0)list->setCurrentRow(0);rename_[i]->setEnabled(list->currentRow()>0);remove_[i]->setEnabled(list->currentRow()>0);
    }
}
bool ProfileSettingsPage::eventFilter(QObject* watched,QEvent* event){
    if(event->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(event);for(std::size_t i=0;i<lists_.size();++i)if(watched==lists_[i]){if(key->key()==Qt::Key_F2){rename(i);return true;}if(key->key()==Qt::Key_Delete){remove(i);return true;}}}return QWidget::eventFilter(watched,event);
}
void ProfileSettingsPage::rename(std::size_t index){
    namespace P=Infrastructure::ProfileConstants;
    auto* selected=lists_[index]->currentItem();if(!selected)return;
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    if(lists_[index]->currentRow()==0){QMessageBox::warning(this,text(P::Title),text(P::SystemRename));return;}
    const auto id=selected->data(Qt::UserRole).toString();bool accepted=false;const auto name=QInputDialog::getText(this,text(P::Rename),text(P::NameLabel),QLineEdit::Normal,selected->text(),&accepted);if(!accepted)return;
    try{if(name.trimmed().compare(text(P::DefaultLabel),Qt::CaseInsensitive)==0)throw std::invalid_argument(P::SystemRename);state_.profiles->rename(static_cast<Infrastructure::ProfileOperation>(index),id,name);}catch(const std::exception& error){QMessageBox::warning(this,text(P::Title),mediaErrorText(translations_,error));}
}
void ProfileSettingsPage::remove(std::size_t index){
    namespace P=Infrastructure::ProfileConstants;
    auto* selected=lists_[index]->currentItem();if(!selected)return;
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    if(lists_[index]->currentRow()==0){QMessageBox::warning(this,text(P::Title),text(P::SystemDelete));return;}
    const auto id=selected->data(Qt::UserRole).toString();QMessageBox dialog(QMessageBox::Warning,text(P::Title),text(P::DeleteConfirm).replace(P::Argument,selected->text()),QMessageBox::Yes|QMessageBox::No,this);dialog.button(QMessageBox::Yes)->setText(text(P::Yes));dialog.button(QMessageBox::No)->setText(text(P::No));dialog.setDefaultButton(QMessageBox::No);if(dialog.exec()!=QMessageBox::Yes)return;
    try{state_.profiles->remove(static_cast<Infrastructure::ProfileOperation>(index),id);}catch(const std::exception& error){QMessageBox::warning(this,text(P::Title),mediaErrorText(translations_,error));}
}
}
