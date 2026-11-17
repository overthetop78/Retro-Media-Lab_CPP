#include "CapturePage.h"
#include "CaptureConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Localization/MediaText.h"
#include "../FlowLayout.h"
#include "../ContentScrollArea.h"
#include "Media/ExplorerSession.h"
#include "Media/MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/ImageFormat.h>
#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
namespace C=CaptureConstants;
namespace M=Infrastructure::MediaCommandConstants;
using Infrastructure::CaptureMode;
CapturePage::CapturePage(CaptureMode mode,ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,std::function<void(const QString&)> navigate,QWidget* parent)
    :QWidget(parent),translations_(translations),state_(state),navigate_(std::move(navigate)) {
    state.ensureMediaOperations(translations);
    session_=mode==CaptureMode::Read?state.reading:state.writing;
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(C::Padding,C::Padding,C::Padding,C::Padding);layout->setSpacing(C::Gap);
    inputs_=new QWidget(this);auto* body=new QVBoxLayout(inputs_);body->setContentsMargins(0,0,0,0);body->setSpacing(C::Gap);
    auto* header=new Card(themes.data(),inputs_);auto* headerRow=new QHBoxLayout(header);
    devices_=new QComboBox(header);devices_->setMinimumWidth(0);devices_->setObjectName(C::DeviceInput);
    profiles_=new QComboBox(header);profileLabel_=new QLabel(header);save_=new QPushButton(header);reset_=new QPushButton(header);
    headerRow->addWidget(devices_,1);headerRow->addWidget(profileLabel_);headerRow->addWidget(profiles_,1);headerRow->addWidget(save_);headerRow->addWidget(reset_);body->addWidget(header);
    auto* sourceCard=new Card(themes.data(),inputs_);auto* sourceRow=new QGridLayout(sourceCard);
    pathLabel_=new QLabel(sourceCard);path_=new QLineEdit(sourceCard);browse_=new QPushButton(sourceCard);
    path_->setObjectName(mode==CaptureMode::Read?C::DirectoryInput:C::SourceInput);
    sourceRow->addWidget(pathLabel_,0,0);sourceRow->addWidget(path_,1,0);sourceRow->addWidget(browse_,1,1);
    nameLabel_=new QLabel(sourceCard);name_=new QLineEdit(sourceCard);name_->setObjectName(C::NameInput);
    if(mode==CaptureMode::Read) {sourceRow->addWidget(nameLabel_,0,C::NameColumn);sourceRow->addWidget(name_,1,C::NameColumn);}
    else {nameLabel_->hide();name_->hide();}
    sourceRow->setColumnStretch(0,1);if(mode==CaptureMode::Read)sourceRow->setColumnStretch(C::NameColumn,1);body->addWidget(sourceCard);
    auto* formatCard=new Card(themes.data(),inputs_);auto* formatRow=new QHBoxLayout(formatCard);
    if(mode==CaptureMode::Read) {raw_=new QRadioButton(formatCard);known_=new QRadioButton(formatCard);formatRow->addWidget(raw_);formatRow->addWidget(known_);}
    formats_=new QComboBox(formatCard);formats_->setMinimumWidth(0);formats_->setObjectName(C::FormatInput);formatRow->addWidget(formats_,1);body->addWidget(formatCard);
    extensions_=new QComboBox(formatCard);formatRow->addWidget(extensions_);
    advancedToggle_=new QToolButton(inputs_);advancedToggle_->setCheckable(true);advancedToggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);body->addWidget(advancedToggle_,0,Qt::AlignLeft);
    advanced_=new Card(themes.data(),inputs_);auto* advancedBody=new QVBoxLayout(advanced_);
    auto* scroll=new ContentScrollArea(C::AdvancedHeight,advanced_);auto* optionHost=new QWidget(scroll);auto* options=new FlowLayout(optionHost);options->setContentsMargins(0,0,0,0);options->setSpacing(C::Gap);
    for(const auto& definition:Infrastructure::captureOptions(mode)) {
        auto* container=new QWidget(optionHost);auto* row=new QHBoxLayout(container);row->setContentsMargins(0,0,0,0);row->setSpacing(C::Gap);
        auto* check=new QCheckBox(container);row->addWidget(check);QLineEdit* value=nullptr;
        if(!definition.flag) {value=new QLineEdit(container);value->setFixedWidth(C::OptionWidth);row->addWidget(value);}
        fields_.push_back({definition,check,value});options->addWidget(container);
        connect(check,&QCheckBox::toggled,this,[this,argument=QString::fromLatin1(definition.argument)](bool enabled) {
            if(enabled)session_->draft.enabled.insert(argument);else session_->draft.enabled.remove(argument);
            if(enabled) {
                if(argument==M::FakeIndex)session_->draft.enabled.remove(M::HardSectors);if(argument==M::HardSectors)session_->draft.enabled.remove(M::FakeIndex);
                if(argument==M::Densel)session_->draft.enabled.remove(M::Tg43);if(argument==M::Tg43)session_->draft.enabled.remove(M::Densel);
            }
            session_->changed();
        });
        if(value)connect(value,&QLineEdit::textEdited,this,[this,argument=QString::fromLatin1(definition.argument)](const QString& text){session_->draft.values[argument]=text;session_->changed();});
    }
    scroll->setContent(optionHost);advancedBody->addWidget(scroll);
    sequencePanel_=new QWidget(advanced_);auto* sequenceRow=new QHBoxLayout(sequencePanel_);sequenceRow->setContentsMargins(0,0,0,0);
    autoNumber_=new QCheckBox(sequencePanel_);sequenceKind_=new QComboBox(sequencePanel_);sequenceKind_->addItems({QString{},QString{}});
    sequenceWidth_=new QSpinBox(sequencePanel_);sequenceWidth_->setRange(1,M::MaximumSequenceWidth);
    sequence_=new QLineEdit(sequencePanel_);sequence_->setFixedWidth(C::SequenceWidth);
    sequenceRow->addWidget(autoNumber_);sequenceRow->addWidget(sequenceKind_);sequenceRow->addWidget(sequenceWidth_);sequenceRow->addWidget(sequence_);sequenceRow->addStretch();
    advancedBody->addWidget(sequencePanel_);sequencePanel_->setVisible(mode==CaptureMode::Read);
    expertLabel_=new QLabel(advanced_);expert_=new QLineEdit(advanced_);advancedBody->addWidget(expertLabel_);advancedBody->addWidget(expert_);body->addWidget(advanced_);
    layout->addWidget(inputs_);layout->addStretch();
    summary_=new QLabel(this);summary_->setWordWrap(true);summary_->setTextFormat(Qt::PlainText);layout->addWidget(summary_);
    auto* footer=new QHBoxLayout;visualize_=new QPushButton(this);explore_=new QPushButton(this);execute_=new QPushButton(this);
    footer->addWidget(visualize_);footer->addWidget(explore_);footer->addStretch();footer->addWidget(execute_);layout->addLayout(footer);
    connect(devices_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0){session_->draft.deviceId=devices_->itemData(index).toString();session_->changed();}});
    connect(formats_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0){session_->draft.formatId=formats_->itemData(index).toString();if(const auto* format=MediaEngine::findImageFormat(session_->draft.formatId.toStdString());format && !format->extensions.empty())session_->draft.extension=QString::fromStdString(format->extensions.front().extension);session_->changed();}});
    connect(path_,&QLineEdit::textEdited,this,[this](const QString& value){if(session_->mode==CaptureMode::Read)session_->draft.directory=value;else {session_->draft.source=value;sourceEdited_=true;}session_->changed();});
    connect(path_,&QLineEdit::editingFinished,this,[this]{if(sourceEdited_){sourceEdited_=false;try{session_->selectSource(path_->text());}catch(const std::exception& error){summary_->setText(mediaErrorText(translations_,error));}}});
    connect(extensions_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0){session_->draft.extension=extensions_->itemData(index).toString();session_->changed();}});
    connect(name_,&QLineEdit::textEdited,this,[this](const QString& value){session_->draft.fileName=value;session_->changed();});
    connect(expert_,&QLineEdit::textEdited,this,[this](const QString& value){session_->draft.expert=value;session_->changed();});
    connect(browse_,&QPushButton::clicked,this,[this]{browse();});connect(execute_,&QPushButton::clicked,this,[this]{execute();});
    connect(advancedToggle_,&QToolButton::toggled,this,[this](bool enabled){session_->draft.advancedOpen=enabled;session_->changed();});
    if(raw_)connect(raw_,&QRadioButton::toggled,this,[this](bool raw){session_->draft.raw=raw;if(!raw && session_->draft.formatId.isEmpty() && formats_->count()>0){session_->draft.formatId=formats_->itemData(0).toString();if(const auto* format=MediaEngine::findImageFormat(session_->draft.formatId.toStdString());format && !format->extensions.empty())session_->draft.extension=QString::fromStdString(format->extensions.front().extension);}session_->changed();});
    connect(autoNumber_,&QCheckBox::toggled,this,[this](bool enabled){session_->draft.autoNumber=enabled;session_->changed();});
    connect(sequence_,&QLineEdit::textEdited,this,[this](const QString& value){session_->draft.sequence=value;session_->changed();});
    connect(sequenceWidth_,&QSpinBox::valueChanged,this,[this](int value){session_->draft.sequenceWidth=value;session_->changed();});
    connect(sequenceKind_,&QComboBox::currentIndexChanged,this,[this](int index){qint64 value=0;if(!Infrastructure::parseCaptureSequence(session_->draft.sequence,session_->draft.alphabetic,value))value=0;session_->draft.alphabetic=index==1;session_->draft.sequence=Infrastructure::captureSequence(value,session_->draft.alphabetic,1);session_->changed();});
    connect(profiles_,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)session_->applyProfile(static_cast<std::size_t>(index));});
    connect(reset_,&QPushButton::clicked,this,[this]{session_->applyProfile(session_->selectedProfile);});connect(save_,&QPushButton::clicked,this,[this]{saveProfile();});
    connect(visualize_,&QPushButton::clicked,this,[this]{openResult(AppConstants::Visualize);});connect(explore_,&QPushButton::clicked,this,[this]{openResult(AppConstants::Explorer);});
    for(auto* combo:{devices_,profiles_,formats_}){combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);combo->setMinimumContentsLength(C::ComboMinimumLength);}
    rebuildFormats();subscription_=session_->subscribe([this]{refresh();});hardwareSubscription_=state.subscribeToHardware([this]{refresh();});engineSubscription_=state.subscribeToEngines([this]{refresh();});
    languageSubscription_=translations.subscribe([this]{rebuildFormats();refresh();});refresh();
}
QString CapturePage::text(const char* key)const{return QString::fromStdString(translations_.text(key));}
void CapturePage::showEvent(QShowEvent* event){QWidget::showEvent(event);refresh();}
void CapturePage::rebuildFormats() {
    const QSignalBlocker blocked(formats_);formats_->clear();
    if(session_->mode==CaptureMode::Write)formats_->addItem(text(C::Automatic),QString{});
    for(const auto& format:MediaEngine::imageFormats())if(session_->mode==CaptureMode::Read?format.supportsPhysicalRead:format.supportsPhysicalWrite) {
        const auto resolver=[this](std::string_view key){return translations_.text(std::string(key),Infrastructure::MediaSessionConstants::MediaDomain);};
        formats_->addItem(QString::fromStdString(MediaEngine::imageFormatFamilyName(format,resolver))+QString::fromUtf8(C::Separator)+QString::fromStdString(MediaEngine::imageFormatName(format,resolver)),QString::fromStdString(format.id));
    }
}
void CapturePage::refresh() {
    const auto& draft=session_->draft;const bool read=session_->mode==CaptureMode::Read;inputs_->setEnabled(!session_->busy());
    {const QSignalBlocker blocked(devices_);devices_->clear();for(const auto& device:state_.hardware().devices)if(device.category==HardwareConstants::Floppy)devices_->addItem(device.name+QString::fromUtf8(C::Separator)+device.address,device.id);
        auto index=devices_->findData(draft.deviceId);if(index<0 && devices_->count()>0){index=0;session_->draft.deviceId=devices_->itemData(index).toString();}devices_->setCurrentIndex(index);}
    {const QSignalBlocker blocked(profiles_);profiles_->clear();for(std::size_t index=0;index<session_->profiles().size();++index)profiles_->addItem(index?session_->profiles()[index].name:text(C::DefaultProfile));profiles_->setCurrentIndex(static_cast<int>(session_->selectedProfile));}
    profileLabel_->setText(text(C::Profile));save_->setText(text(C::Save));reset_->setText(text(C::Reset));
    pathLabel_->setText(text(read?C::Folder:C::Source));nameLabel_->setText(text(C::Name));browse_->setText(text(C::Browse));
    const auto set=[](QLineEdit* edit,const QString& value){if(edit->text()!=value)edit->setText(value);};set(path_,read?draft.directory:draft.source);set(name_,draft.fileName);set(expert_,draft.expert);set(sequence_,draft.sequence);
    {const QSignalBlocker blocked(formats_);formats_->setCurrentIndex(formats_->findData(draft.formatId));}formats_->setEnabled(!read || !draft.raw);
    formats_->setVisible(!read || !draft.raw);
    {const QSignalBlocker blocked(extensions_);extensions_->clear();if(const auto* format=MediaEngine::findImageFormat(draft.formatId.toStdString()))for(const auto& extension:format->extensions)extensions_->addItem(QString::fromStdString(extension.extension),QString::fromStdString(extension.extension));extensions_->setCurrentIndex(extensions_->findData(draft.extension));}
    extensions_->setVisible(read && !draft.raw);extensions_->setAccessibleName(text(C::Extension));
    if(raw_){const QSignalBlocker a(raw_),b(known_);raw_->setText(text(C::Raw));known_->setText(text(C::Known));raw_->setChecked(draft.raw);known_->setChecked(!draft.raw);}
    {const QSignalBlocker blocked(advancedToggle_);advancedToggle_->setText(text(C::Advanced));advancedToggle_->setChecked(draft.advancedOpen);}advanced_->setVisible(draft.advancedOpen);advancedToggle_->setArrowType(draft.advancedOpen?Qt::DownArrow:Qt::RightArrow);
    for(const auto& field:fields_){const QSignalBlocker blocked(field.check);field.check->setText(text(field.definition.label));field.check->setChecked(draft.enabled.contains(field.definition.argument));if(field.value){field.value->setEnabled(field.check->isChecked());set(field.value,draft.values.value(field.definition.argument));}}
    {const QSignalBlocker a(autoNumber_),b(sequenceKind_),c(sequenceWidth_);autoNumber_->setText(text(C::AutoNumber));autoNumber_->setChecked(draft.autoNumber);sequenceKind_->setItemText(0,text(C::Digits));sequenceKind_->setItemText(1,text(C::Letters));sequenceKind_->setCurrentIndex(draft.alphabetic?1:0);sequenceWidth_->setValue(draft.sequenceWidth);}
    expertLabel_->setText(text(C::Expert));execute_->setText(text(session_->busy()?C::Stop:C::Execute));execute_->setEnabled(session_->busy() || state_.greaseweazleTools->activeOperation().isEmpty());
    auto summary=text(session_->execution().summaryKey.toUtf8().constData());for(int index=0;index<session_->execution().summaryArguments.size();++index)summary.replace(QString::fromLatin1(C::Argument).arg(index),session_->execution().summaryArguments[index]);summary_->setText(summary);
    if(!session_->cleanupError().isEmpty())summary_->setText(summary+QLatin1Char(C::NewLine)+mediaText(translations_,{C::CleanupFailed,{session_->cleanupPath().toStdString(),session_->cleanupError().toStdString()},{}}));
    visualize_->setText(text(C::OpenVisual));explore_->setText(text(C::OpenExplorer));const bool hasResult=read?!session_->completedPath().isEmpty():QFileInfo::exists(draft.source);visualize_->setVisible(hasResult);explore_->setVisible(hasResult);
    if(isVisible() && state_.greaseweazleTools->activeOperation().isEmpty())try {const auto command=session_->command(state_.hardware(),state_.engines(),QCoreApplication::applicationDirPath());state_.console.setContent(command.displayText(),state_.console.output);}catch(const std::exception&){state_.console.setContent({},state_.console.output);}
}
void CapturePage::browse() {
    if(session_->mode==CaptureMode::Read){const auto folder=QFileDialog::getExistingDirectory(this,text(C::Folder),session_->draft.directory);if(!folder.isEmpty()){session_->draft.directory=folder;session_->changed();}}
    else {const auto path=QFileDialog::getOpenFileName(this,text(C::Source),state_.defaultImageDirectory());if(!path.isEmpty())try{session_->selectSource(path);}catch(const std::exception& error){summary_->setText(mediaErrorText(translations_,error));}}
}
bool CapturePage::confirm(const QString& title,const QString& message){QMessageBox box(QMessageBox::Warning,title,message,QMessageBox::Yes|QMessageBox::No,this);box.setTextFormat(Qt::PlainText);box.setDefaultButton(QMessageBox::No);box.button(QMessageBox::Yes)->setText(text(C::Yes));box.button(QMessageBox::No)->setText(text(C::No));return box.exec()==QMessageBox::Yes;}
void CapturePage::execute() {
    if(session_->busy()){if(confirm(text(C::StopTitle),text(C::StopConfirm)))session_->stop();return;}
    try {
        session_->command(state_.hardware(),state_.engines(),QCoreApplication::applicationDirPath());bool overwrite=false;
        if(session_->mode==CaptureMode::Read && QFileInfo::exists(session_->targetPath())) {
            QMessageBox conflict(QMessageBox::Question,text(C::FileExists),session_->targetPath(),QMessageBox::NoButton,this);conflict.setTextFormat(Qt::PlainText);
            auto* replace=conflict.addButton(text(C::Overwrite),QMessageBox::DestructiveRole);auto* next=conflict.addButton(text(C::NextNumber),QMessageBox::ActionRole);auto* edit=conflict.addButton(text(C::EditName),QMessageBox::RejectRole);conflict.setDefaultButton(edit);conflict.exec();
            if(conflict.clickedButton()==next)session_->nextAvailable();else if(conflict.clickedButton()==replace)overwrite=true;else return;
        }
        if(session_->mode==CaptureMode::Write) {
            auto prompt=text(C::WriteConfirm);const auto device=devices_->currentText();const QStringList arguments{session_->draft.source,formats_->currentText(),device,text(session_->draft.enabled.contains(M::NoVerify)?C::VerifyOff:C::VerifyOn)};
            for(int index=0;index<arguments.size();++index)prompt.replace(QString::fromLatin1(C::Argument).arg(index),arguments[index]);if(!confirm(text(C::WriteTitle),prompt))return;
        }
        session_->start(state_.hardware(),state_.engines(),QCoreApplication::applicationDirPath(),overwrite);
    }catch(const std::exception& error){auto message=mediaErrorText(translations_,error);message.replace(QString::fromLatin1(C::Argument).arg(0),text(EngineConstants::InternalKey));summary_->setText(message);}
}
void CapturePage::saveProfile(){bool ok=false;const auto name=QInputDialog::getText(this,text(C::SaveProfile),text(C::ProfileName),QLineEdit::Normal,{},&ok);if(!ok)return;if(name.trimmed().compare(text(C::DefaultProfile),Qt::CaseInsensitive)==0){summary_->setText(text(C::ReservedProfile));return;}bool replace=false;for(const auto& profile:session_->profiles())if(profile.name.compare(name.trimmed(),Qt::CaseInsensitive)==0){replace=confirm(text(C::SaveProfile),text(C::ReplaceProfile));if(!replace)return;}try{session_->saveProfile(name,replace);}catch(const std::exception& error){summary_->setText(mediaErrorText(translations_,error));}}
void CapturePage::openResult(const char* page){const auto path=session_->mode==CaptureMode::Read?session_->completedPath():session_->draft.source;if(path.isEmpty())return;if(!state_.explorer)state_.explorer=std::make_shared<Infrastructure::ExplorerSession>();state_.explorer->open(path);navigate_(QString::fromLatin1(page));}
}
