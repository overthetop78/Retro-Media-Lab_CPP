#include "ConversionPage.h"
#include "ConversionPageConstants.h"
#include "CaptureConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include "Media/MediaSessionConstants.h"
#include "../ContentScrollArea.h"
#include "../FlowLayout.h"
#include <QCheckBox>
#include <QComboBox>
#include <QInputDialog>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
namespace C=ConversionPageConstants;
namespace M=Infrastructure::MediaCommandConstants;
namespace E=MediaEngine;
ConversionPage::ConversionPage(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),translations_(translations),state_(state) {
    state.ensureMediaOperations(translations);session_=state.converting;
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Gap);
    inputs_=new QWidget(this);auto* body=new QVBoxLayout(inputs_);body->setContentsMargins(0,0,0,0);body->setSpacing(C::Gap);
    auto* profileCard=new Card(themes.data(),inputs_);auto* profileRow=new QHBoxLayout(profileCard);profileLabel_=new QLabel(profileCard);profiles_=new QComboBox(profileCard);save_=new QPushButton(profileCard);reset_=new QPushButton(profileCard);profileRow->addWidget(profileLabel_);profileRow->addWidget(profiles_,1);profileRow->addWidget(save_);profileRow->addWidget(reset_);body->addWidget(profileCard);
    auto* sourceCard=new Card(themes.data(),inputs_);auto* row=new QGridLayout(sourceCard);
    sourceLabel_=new QLabel(sourceCard);source_=new QLineEdit(sourceCard);source_->setObjectName(C::SourceInput);browse_=new QPushButton(sourceCard);
    nameLabel_=new QLabel(sourceCard);name_=new QLineEdit(sourceCard);name_->setObjectName(C::NameInput);tags_=new QCheckBox(sourceCard);
    row->addWidget(sourceLabel_,0,0);row->addWidget(source_,1,0);row->addWidget(browse_,1,1);row->addWidget(nameLabel_,0,C::NameColumn);row->addWidget(name_,1,C::NameColumn);row->addWidget(tags_,1,C::TagsColumn);row->setColumnStretch(0,C::SourceStretch);row->setColumnStretch(C::NameColumn,1);body->addWidget(sourceCard);
    auto* selectors=new QHBoxLayout;auto* formatCard=new Card(themes.data(),inputs_);auto* formatLayout=new QVBoxLayout(formatCard);
    formatLabel_=new QLabel(formatCard);rare_=new QCheckBox(formatCard);formats_=new QTreeWidget(formatCard);formats_->setHeaderHidden(true);formats_->setObjectName(C::FormatTree);formats_->setUniformRowHeights(true);
    formatLayout->addWidget(formatLabel_);formatLayout->addWidget(rare_);formatLayout->addWidget(formats_,1);selectors->addWidget(formatCard,1);
    auto* selectedCard=new Card(themes.data(),inputs_);auto* selectedLayout=new QVBoxLayout(selectedCard);selectedLabel_=new QLabel(selectedCard);selected_=new QListWidget(selectedCard);selected_->setWordWrap(true);
    selectedLayout->addWidget(selectedLabel_);selectedLayout->addWidget(selected_,1);selectors->addWidget(selectedCard,1);body->addLayout(selectors,1);
    advancedToggle_=new QToolButton(inputs_);advancedToggle_->setCheckable(true);advancedToggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);body->addWidget(advancedToggle_,0,Qt::AlignLeft);
    advanced_=new Card(themes.data(),inputs_);auto* advancedLayout=new QVBoxLayout(advanced_);auto* scroll=new ContentScrollArea(C::AdvancedHeight,advanced_);auto* optionsHost=new QWidget(scroll);auto* options=new FlowLayout(optionsHost);options->setContentsMargins(0,0,0,0);options->setSpacing(C::Gap);
    for(const auto& definition:Infrastructure::conversionOptions()) {
        auto* field=new QWidget(optionsHost);auto* column=new QVBoxLayout(field);column->setContentsMargins(0,0,0,0);auto* check=new QCheckBox(field);column->addWidget(check);QLineEdit* value=nullptr;
        if(!definition.flag){value=new QLineEdit(field);value->setMaximumWidth(C::OptionWidth);column->addWidget(value);connect(value,&QLineEdit::textEdited,this,[this,definition](const QString& input){session_->draft.values.insert(definition.argument,input);session_->changed();});}
        options->addWidget(field);fields_.push_back({definition,check,value});connect(check,&QCheckBox::toggled,this,[this,definition](bool enabled){if(enabled)session_->draft.enabled.insert(definition.argument);else session_->draft.enabled.remove(definition.argument);session_->changed();});
    }
    scroll->setWidget(optionsHost);advancedLayout->addWidget(scroll);expertLabel_=new QLabel(advanced_);expert_=new QLineEdit(advanced_);advancedLayout->addWidget(expertLabel_);advancedLayout->addWidget(expert_);body->addWidget(advanced_);layout->addWidget(inputs_,1);
    auto* footer=new QHBoxLayout;summary_=new QLabel(this);summary_->setWordWrap(true);progress_=new QProgressBar(this);progress_->setRange(0,C::ProgressMaximum);execute_=new QPushButton(this);footer->addWidget(summary_,1);footer->addWidget(progress_);footer->addWidget(execute_);layout->addLayout(footer);
    connect(browse_,&QPushButton::clicked,this,[this]{const auto path=QFileDialog::getOpenFileName(this,text(C::Source),source_->text());if(!path.isEmpty()){source_->setText(path);openSource();}});
    connect(source_,&QLineEdit::editingFinished,this,[this]{if(source_->text()!=session_->draft.source)openSource();});
    connect(name_,&QLineEdit::textEdited,this,[this](const QString& value){session_->draft.name=value;session_->changed();});
    connect(tags_,&QCheckBox::toggled,this,[this](bool enabled){session_->draft.addTags=enabled;session_->changed();});
    connect(rare_,&QCheckBox::toggled,this,[this]{rebuild_=true;refresh();});
    connect(formats_,&QTreeWidget::itemChanged,this,[this](QTreeWidgetItem* item){if(item->data(0,C::ExtensionRole).isValid() && item->checkState(0)==Qt::Checked){const QSignalBlocker blocked(formats_);item->parent()->setCheckState(0,Qt::Checked);}selectFormats();});
    connect(advancedToggle_,&QToolButton::toggled,this,[this](bool enabled){session_->draft.advancedOpen=enabled;session_->changed();});
    connect(expert_,&QLineEdit::textEdited,this,[this](const QString& value){session_->draft.expert=value;session_->changed();});
    connect(execute_,&QPushButton::clicked,this,[this]{execute();});
    connect(profiles_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)session_->applyProfile(static_cast<std::size_t>(index));});
    connect(save_,&QPushButton::clicked,this,[this]{saveProfile();});connect(reset_,&QPushButton::clicked,this,[this]{session_->applyProfile(session_->selectedProfile);});
    subscription_=session_->subscribe([this]{refresh();});
    engineSubscription_=state.subscribeToEngines([this]{if(!session_->busy() && !session_->draft.source.isEmpty())openSource();});
    settingsSubscription_=state.subscribeToConversion([this]{refresh();});languageSubscription_=translations.subscribe([this]{rebuild_=true;refresh();});refresh();
}
QString ConversionPage::text(const char* key)const{return QString::fromStdString(translations_.text(key));}
void ConversionPage::showEvent(QShowEvent* event){QWidget::showEvent(event);refresh();}
void ConversionPage::openSource() {
    try {session_->open(source_->text(),state_.engines().engineFor(EngineConstants::Greaseweazle,EngineConstants::Conversion)==EngineConstants::Internal);}
    catch(const std::exception& error){summary_->setText(mediaErrorText(translations_,error));}
}
void ConversionPage::rebuildFormats() {
    const QSignalBlocker blocked(formats_);formats_->clear();displayedIds_.clear();
    QMap<QString,QTreeWidgetItem*> families;
    const auto resolver=[this](std::string_view key){return translations_.text(std::string(key),Infrastructure::MediaSessionConstants::MediaDomain);};
    for(const auto& format:session_->formats()) {
        displayedIds_.push_back(format.id);
        if(!format.isCommon && !rare_->isChecked())continue;
        const auto family=QString::fromStdString(E::imageFormatFamilyName(format,resolver));
        auto*& parent=families[family];if(!parent){parent=new QTreeWidgetItem(formats_,{family});parent->setExpanded(true);}
        auto* item=new QTreeWidgetItem(parent,{QString::fromStdString(E::imageFormatName(format,resolver))});item->setData(0,C::FormatRole,QString::fromStdString(format.id));item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        const auto selected=std::find_if(session_->draft.selections.begin(),session_->draft.selections.end(),[&](const auto& selection){return selection.formatId==format.id;});
        item->setCheckState(0,selected==session_->draft.selections.end()?Qt::Unchecked:Qt::Checked);
        for(const auto& extension:format.extensions){auto* child=new QTreeWidgetItem(item,{QString::fromStdString(extension.extension)});child->setData(0,C::ExtensionRole,QString::fromStdString(extension.extension));child->setFlags(child->flags()|Qt::ItemIsUserCheckable);child->setCheckState(0,selected!=session_->draft.selections.end() && std::find(selected->explicitExtensions.begin(),selected->explicitExtensions.end(),extension.extension)!=selected->explicitExtensions.end()?Qt::Checked:Qt::Unchecked);}
    }rebuild_=false;
}
void ConversionPage::selectFormats() {
    // Keep selections of hidden rare formats; a display filter must not alter a conversion request.
    auto selections=session_->draft.selections;
    for(int family=0;family<formats_->topLevelItemCount();++family)for(int index=0;index<formats_->topLevelItem(family)->childCount();++index) {
        auto* item=formats_->topLevelItem(family)->child(index);const auto id=item->data(0,C::FormatRole).toString().toStdString();
        std::erase_if(selections,[&](const auto& selection){return selection.formatId==id;});
        if(item->checkState(0)!=Qt::Checked)continue;
        E::ConversionSelection selection{id,{}};for(int ext=0;ext<item->childCount();++ext)if(item->child(ext)->checkState(0)==Qt::Checked)selection.explicitExtensions.push_back(item->child(ext)->data(0,C::ExtensionRole).toString().toStdString());
        selections.push_back(std::move(selection));
    }session_->draft.selections=std::move(selections);selectionSignature_=selectionSignature();session_->changed();
}
QString ConversionPage::selectionSignature()const{QString result;for(const auto& selection:session_->draft.selections){result+=QString::fromStdString(selection.formatId)+QString::fromLatin1(C::NewLine);for(const auto& extension:selection.explicitExtensions)result+=QString::fromStdString(extension)+QString::fromLatin1(C::NewLine);}return result;}
void ConversionPage::refresh() {
    const bool busy=session_->busy();inputs_->setEnabled(!busy);execute_->setText(text(busy?C::Stop:C::Execute));
    sourceLabel_->setText(text(C::Source));nameLabel_->setText(text(C::Name));browse_->setText(text(C::Browse));tags_->setText(text(C::AddTags));rare_->setText(text(C::ShowRare));formatLabel_->setText(text(C::Formats));selectedLabel_->setText(text(C::Selected));expertLabel_->setText(text(C::Expert));advancedToggle_->setText(text(C::Advanced));
    profileLabel_->setText(text(C::Profile));save_->setText(text(C::Save));reset_->setText(text(C::Reset));
    {const QSignalBlocker blocked(profiles_);profiles_->clear();for(const auto& profile:session_->profiles())profiles_->addItem(profile.system?text(C::DefaultProfile):profile.name,profile.id);profiles_->setCurrentIndex(static_cast<int>(session_->selectedProfile));}
    const auto set=[](QLineEdit* edit,const QString& value){if(edit->text()!=value)edit->setText(value);};
    set(source_,session_->draft.source);set(name_,session_->draft.name);set(expert_,session_->draft.expert);
    {const QSignalBlocker blocked(tags_);tags_->setChecked(session_->draft.addTags);}
    {const QSignalBlocker blocked(advancedToggle_);advancedToggle_->setChecked(session_->draft.advancedOpen);advanced_->setVisible(session_->draft.advancedOpen);}
    advancedToggle_->setArrowType(session_->draft.advancedOpen?Qt::DownArrow:Qt::RightArrow);
    for(auto& field:fields_){field.check->setText(text(field.definition.argument==M::OutputTracks?C::OutputTracks:field.definition.label));const QSignalBlocker blocked(field.check);field.check->setChecked(session_->draft.enabled.contains(field.definition.argument));if(field.value){set(field.value,session_->draft.values.value(field.definition.argument));field.value->setEnabled(field.check->isChecked());}}
    const auto signature=selectionSignature();
    std::vector<std::string> ids;for(const auto& format:session_->formats())ids.push_back(format.id);if(rebuild_ || ids!=displayedIds_ || signature!=selectionSignature_){rebuildFormats();selectionSignature_=signature;}
    progress_->setVisible(session_->converting());progress_->setValue(static_cast<int>(session_->progress()));
    QStringList messages;for(const auto& message:session_->messages())messages.push_back(mediaText(translations_,message));
    if(session_->error())summary_->setText(mediaText(translations_,*session_->error()));else if(!messages.isEmpty())summary_->setText(messages.back());
    if(!busy)try {
        const auto planned=session_->plan(state_.conversion());selected_->clear();
        for(const auto& target:planned){const auto key=target.fidelity==E::ConversionFidelityLevel::PreservedFlux?C::Flux:target.fidelity==E::ConversionFidelityLevel::ReconstructedTracks?C::Tracks:C::Sector;selected_->addItem(QString::fromStdU16String(target.outputPath.filename().u16string())+QString::fromUtf8(C::Separator)+text(key));}
        if(isVisible() && state_.greaseweazleTools->activeOperation().isEmpty())state_.console.setContent(session_->preview(state_.hardware(),state_.engines(),state_.conversion(),QCoreApplication::applicationDirPath()),state_.console.output);
        if(!session_->error() && messages.isEmpty())summary_->clear();
    }catch(const std::exception& error){selected_->clear();if(!session_->error())summary_->setText(mediaErrorText(translations_,error));if(isVisible() && state_.greaseweazleTools->activeOperation().isEmpty())state_.console.setContent({},state_.console.output);}
    if(session_->awaitingLosses() && !showingLosses_){showingLosses_=true;QTimer::singleShot(0,this,[this]{askLosses();});}
}
void ConversionPage::askLosses() {
    QStringList losses;for(const auto& loss:session_->losses())losses.push_back(mediaText(translations_,loss));
    QMessageBox dialog(QMessageBox::Warning,text(C::LossTitle),text(C::LossConfirm).replace(QString::fromLatin1(C::Argument),losses.join(QString::fromLatin1(C::NewLine))),QMessageBox::Yes|QMessageBox::No,this);
    dialog.setDefaultButton(QMessageBox::No);const bool accepted=dialog.exec()==QMessageBox::Yes;showingLosses_=false;session_->acceptLosses(accepted);
}
void ConversionPage::execute() {
    if(session_->busy()){QMessageBox dialog(QMessageBox::Question,text(C::StopTitle),text(C::StopConfirm),QMessageBox::Yes|QMessageBox::No,this);dialog.setDefaultButton(QMessageBox::No);if(dialog.exec()==QMessageBox::Yes)session_->cancel();return;}
    try {
        auto decision=Infrastructure::ConversionConflict::Overwrite;QStringList existing;
        for(const auto& target:session_->plan(state_.conversion()))if(QFileInfo::exists(QString::fromStdU16String(target.outputPath.u16string())))existing.push_back(QString::fromStdU16String(target.outputPath.u16string()));
        if(!existing.isEmpty()){
            QMessageBox dialog(QMessageBox::Question,text(C::Conflict),existing.join(QString::fromLatin1(C::NewLine)),QMessageBox::NoButton,this);
            auto* overwrite=dialog.addButton(text(C::Overwrite),QMessageBox::DestructiveRole);auto* skip=dialog.addButton(text(C::Skip),QMessageBox::ActionRole);auto* number=dialog.addButton(text(C::Number),QMessageBox::ActionRole);auto* cancel=dialog.addButton(text(C::Cancel),QMessageBox::RejectRole);dialog.setDefaultButton(cancel);dialog.exec();
            if(dialog.clickedButton()==cancel)return;if(dialog.clickedButton()==number)decision=Infrastructure::ConversionConflict::Number;else if(dialog.clickedButton()==skip)decision=Infrastructure::ConversionConflict::Skip;else if(dialog.clickedButton()!=overwrite)return;
        }
        state_.console.setContent(session_->preview(state_.hardware(),state_.engines(),state_.conversion(),QCoreApplication::applicationDirPath()),{});
        session_->start(state_.hardware(),state_.engines(),state_.conversion(),QCoreApplication::applicationDirPath(),decision);
    }catch(const std::exception& error){QMessageBox::warning(this,text(C::Title),mediaErrorText(translations_,error));}
}
void ConversionPage::saveProfile(){
    bool accepted=false;const auto name=QInputDialog::getText(this,text(C::SaveProfile),text(C::ProfileName),QLineEdit::Normal,{},&accepted);if(!accepted)return;
    if(name.trimmed().compare(text(C::DefaultProfile),Qt::CaseInsensitive)==0){QMessageBox::warning(this,text(C::Profile),text(C::ReservedProfile));return;}
    bool replace=false;for(const auto& profile:session_->profiles())if(profile.name.compare(name.trimmed(),Qt::CaseInsensitive)==0){QMessageBox dialog(QMessageBox::Question,text(C::SaveProfile),text(C::ReplaceProfile),QMessageBox::Yes|QMessageBox::No,this);dialog.setDefaultButton(QMessageBox::No);replace=dialog.exec()==QMessageBox::Yes;if(!replace)return;}
    try{session_->saveProfile(name,replace);}catch(const std::exception& error){QMessageBox::warning(this,text(C::Profile),mediaErrorText(translations_,error));}
}
}
