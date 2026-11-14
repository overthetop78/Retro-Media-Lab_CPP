#include "FloppySettingsPage.h"
#include "HostToolsSettingsWidget.h"
#include "KryoFluxSettingsWidget.h"
#include "../ThemeIcons.h"
#include "Shared/Controls/Card.h"
#include "Shared/Controls/StatusIndicator.h"
#include "Logging/DiagnosticLog.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QAbstractButton>
#include <QMap>
#include <QPushButton>
#include "../ContentScrollArea.h"
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>
namespace RetroMediaLab {
using namespace HardwareConstants;
struct FloppySettingsPage::ReaderRow {
    HardwareDevice device;
    int number=0;
    QComboBox *size,*density,*rpm,*selection=nullptr;
    QLabel *port,*reader,*sizeLabel,*densityLabel,*speedLabel,*configured;
    StatusIndicator* availability;
    QPushButton *save,*forget;
};
FloppySettingsPage::FloppySettingsPage(ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),themes_(themes),translations_(translations),state_(state),discovery_(std::make_unique<GreaseweazleDiscovery>(state.logsDirectory,[&state]{return state.logging();})){
    setObjectName(Floppy);auto* outer=new QVBoxLayout(this);outer->setContentsMargins(Padding,Padding,Padding,Padding);outer->setSpacing(RowGap);
    auto* controllerType=new QHBoxLayout;backendLabel_=new QLabel(this);backend_=new QComboBox(this);backend_->setObjectName(FloppyBackendInput);
    backend_->addItem(Greaseweazle,Greaseweazle);backend_->addItem(KryoFlux,KryoFlux);backend_->setCurrentIndex(backend_->findData(state.hardware().floppyBackend));
    backendLabel_->setBuddy(backend_);controllerType->addWidget(backendLabel_);controllerType->addWidget(backend_);
    for(const auto& edit:state.floppyReaderEdits)if(edit.id.isEmpty())drafts_.push_back(edit);
    auto* actions=controllerType;outer->addLayout(actions);scan_=new QPushButton(this);scan_->setObjectName(FloppyScan);add_=new QPushButton(this);add_->setObjectName(FloppyAdd);
    for(auto* button:{scan_,add_}){button->setAutoDefault(false);actions->addWidget(button);}controllers_=new QComboBox(this);controllers_->setObjectName(SelectedController);actions->addWidget(controllers_);actions->addStretch();
    auto* scroll=new ContentScrollArea(RowsMaximumHeight,this);readerScroll_=scroll;
    auto* host=new QWidget(scroll);rowsLayout_=new QVBoxLayout(host);rowsLayout_->setContentsMargins(0,0,0,0);scroll->setContent(host);outer->addWidget(scroll);
    persistence_=new QLabel(this);persistence_->setWordWrap(true);outer->addWidget(persistence_);
    tools_=new HostToolsSettingsWidget(translations,state,this);outer->addWidget(tools_);
    kryoTools_=new KryoFluxSettingsWidget(translations,state,this);outer->addWidget(kryoTools_);
    status_=new QLabel(this);status_->setObjectName(Status);status_->setWordWrap(true);status_->setTextFormat(Qt::PlainText);outer->addWidget(status_);outer->addStretch();
    connect(scan_,&QPushButton::clicked,this,[this]{scan();});connect(add_,&QPushButton::clicked,this,[this]{addReader();});
    connect(backend_,&QComboBox::currentIndexChanged,this,[this]{rememberReaderEdits();auto settings=state_.hardware();settings.floppyBackend=backend_->currentData().toString();state_.setHardware(std::move(settings));statusKey_.clear();status_->clear();rebuildRows();});
    subscription_=translations.subscribe([this]{refreshTexts();});rebuildRows();refreshTexts();
}
FloppySettingsPage::~FloppySettingsPage()=default;
void FloppySettingsPage::synchronizeState(){rememberReaderEdits();tools_->synchronizeState();kryoTools_->synchronizeState();}
void FloppySettingsPage::rememberReaderEdits(){
    for(const auto& row:rows_){
        auto device=row->device;device.size=row->size->currentData().toString();device.density=row->density->currentData().toString();device.rpm=row->rpm->currentData().toInt();if(row->selection)device.selection=row->selection->currentData().toString();
        state_.floppyReaderEdits.removeIf([&](const auto& edit){return edit.backend==device.backend&&edit.controller==device.controller&&edit.id==device.id;});
        state_.floppyReaderEdits.push_back(std::move(device));
    }
}
void FloppySettingsPage::failure(const char* key){
    statusKey_=QString::fromUtf8(key);auto text=QString::fromStdString(translations_.text(key));text.replace(Argument,backend_->currentData().toString());status_->setText(text);scan_->setEnabled(true);backend_->setEnabled(true);add_->setEnabled(true);
    writeDiagnostic(state_.logsDirectory,DiagnosticSeverity::Error,text,DiagnosticContext,QString::fromStdString(translations_.language()),DiagnosticProcess);
}
void FloppySettingsPage::rebuildRows(){
    if(rowsContainer_){rowsLayout_->removeWidget(rowsContainer_);rowsContainer_->hide();rowsContainer_->deleteLater();}
    rowsContainer_=new QWidget(this);rowsContainer_->setObjectName(FloppyRows);auto* layout=new QVBoxLayout(rowsContainer_);layout->setContentsMargins(0,0,0,0);layout->setSpacing(RowGap);rowsLayout_->addWidget(rowsContainer_);rows_.clear();
    const auto selectedBackend=backend_->currentData().toString();
    tools_->setVisible(selectedBackend==Greaseweazle);kryoTools_->setVisible(selectedBackend==KryoFlux);
    QVector<HardwareDevice> readers;
    for(const auto& device:state_.hardware().devices)if(device.category==Floppy&&device.backend==selectedBackend)readers.push_back(device);
    for(const auto& draft:drafts_)if(draft.backend==selectedBackend)readers.push_back(draft);
    for(const auto& host:detected_)if(host.backend==selectedBackend&&std::none_of(readers.cbegin(),readers.cend(),[&](const auto& reader){return reader.controller==host.id;})){
        HardwareDevice device;device.category=Floppy;device.backend=selectedBackend;device.controller=host.id;device.address=host.address;device.available=true;readers.push_back(device);
    }
    for(auto& device:readers)for(const auto& edit:state_.floppyReaderEdits)if(edit.backend==device.backend&&edit.controller==device.controller&&edit.id==device.id){device.size=edit.size;device.density=edit.density;device.rpm=edit.rpm;device.selection=edit.selection;break;}
    const auto current=controllers_->currentData().toString();const QSignalBlocker blocked(controllers_);controllers_->clear();
    for(const auto& device:readers)if(controllers_->findData(device.controller)<0)controllers_->addItem(selectedBackend+Space+device.address,device.controller);
    if(selectedBackend==KryoFlux&&controllers_->count()==0)controllers_->addItem(KryoFlux,DefaultKryoController);
    if(controllers_->findData(current)>=0)controllers_->setCurrentIndex(controllers_->findData(current));controllers_->setVisible(controllers_->count()>1);
    QMap<QString,int> numbers;
    for(const auto& device:readers){
        auto row=std::make_shared<ReaderRow>();row->device=device;row->number=++numbers[device.controller];
        auto* card=new Card(themes_.data(),rowsContainer_);card->setObjectName(FloppyRow);auto* line=new QHBoxLayout(card);line->setContentsMargins(RowPadding,RowPadding,RowPadding,RowPadding);line->setSpacing(Gap);
        row->port=new QLabel(device.backend==KryoFlux?QString(UsbAddress):device.address,card);row->port->setToolTip(device.controller);row->port->setMinimumWidth(device.backend==KryoFlux?UsbPortWidth:PortWidth);line->addWidget(row->port);
        row->reader=new QLabel(card);row->reader->setMinimumWidth(device.backend==KryoFlux?0:ReaderWidth);line->addWidget(row->reader);
        if(device.backend==KryoFlux){row->selection=new QComboBox(card);row->selection->setObjectName(KryoSelectionInput);row->selection->setFixedWidth(DriveSelectionWidth);row->selection->addItem(Select0,Select0);row->selection->addItem(Select1,Select1);row->selection->setCurrentIndex(row->selection->findData(device.selection.isEmpty()?QString(Select0):device.selection));line->addWidget(row->selection);}
        row->sizeLabel=new QLabel(card);line->addWidget(row->sizeLabel);row->size=new QComboBox(card);row->size->setObjectName(FloppySize);row->size->setFixedWidth(SizeWidth);
        for(const auto* value:{Size3,DefaultSize,Size525,Size8})row->size->addItem(value,value);row->size->setCurrentIndex(row->size->findData(device.size));line->addWidget(row->size);
        row->densityLabel=new QLabel(card);line->addWidget(row->densityLabel);row->density=new QComboBox(card);row->density->setObjectName(FloppyDensity);row->density->setFixedWidth(DensityWidth);
        for(const auto* value:{UnknownValue,DD,HD,ED})row->density->addItem(value,value);row->density->setCurrentIndex(row->density->findData(device.density));line->addWidget(row->density);
        row->speedLabel=new QLabel(card);line->addWidget(row->speedLabel);row->rpm=new QComboBox(card);row->rpm->setObjectName(FloppyRpm);row->rpm->setFixedWidth(RpmWidth);
        row->rpm->addItem(Empty,NoRpm);row->rpm->addItem(Speed300,Rpm300);row->rpm->addItem(Speed360,Rpm360);row->rpm->setCurrentIndex(row->rpm->findData(device.rpm));line->addWidget(row->rpm);
        row->availability=new StatusIndicator(card);line->addWidget(row->availability);row->configured=new QLabel(card);row->configured->setWordWrap(true);line->addWidget(row->configured);line->addStretch();
        row->save=new QPushButton(card);row->save->setObjectName(FloppySave);row->forget=new QPushButton(card);row->forget->setObjectName(FloppyForget);
        for(auto* button:{row->save,row->forget}){button->setAutoDefault(false);button->setFixedWidth(IconButtonWidth);line->addWidget(button);}
        connect(row->save,&QPushButton::clicked,this,[this,row]{saveReader(row);});connect(row->forget,&QPushButton::clicked,this,[this,row]{forgetReader(row);});
        rows_.push_back(row);layout->addWidget(card);
    }
    layout->addStretch();refreshTexts();readerScroll_->setVisible(!rows_.isEmpty());
}
void FloppySettingsPage::refreshTexts(){
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};backendLabel_->setText(text(ControllerTypeKey));scan_->setText(text(ScanKey));add_->setText(text(AddDriveKey));persistence_->setText(text(PersistenceKey));
    if(!statusKey_.isEmpty())status_->setText(QString::fromStdString(translations_.text(statusKey_.toStdString())).replace(Argument,backend_->currentData().toString()));
    for(auto& row:rows_){
        row->reader->setText(row->device.backend==KryoFlux?text(DriveLabelKey):text(ReaderNumberKey).replace(Argument,QString::number(row->number)));row->sizeLabel->setText(text(SizeShortKey));row->densityLabel->setText(text(DensityShortKey));row->speedLabel->setText(text(SpeedShortKey));
        const QSignalBlocker block(row->density);row->density->setItemText(row->density->findData(UnknownValue),text(UnknownKey));
        row->availability->setStatus(text(row->device.available?AvailableStateKey:UnavailableStateKey),themes_.data()->color(row->device.available?SuccessBrush:MutedBrush));
        row->configured->setText(text(row->device.id.isEmpty()?NotConfiguredKey:ConfiguredKey));
        row->save->setIcon(themeIcon(SaveIcon,themes_.data()->color(MutedBrush)));row->save->setToolTip(text(SaveKey));row->save->setAccessibleName(text(SaveKey));
        row->forget->setIcon(themeIcon(TrashIcon,themes_.data()->color(ErrorBrush)));row->forget->setToolTip(text(ForgetKey));row->forget->setAccessibleName(text(ForgetKey));
    }
}
void FloppySettingsPage::addReader(){
    const auto controller=controllers_->currentData().toString();if(controller.isEmpty()){failure(ControllerRequiredKey);return;}
    const auto count=std::count_if(state_.hardware().devices.cbegin(),state_.hardware().devices.cend(),[&](const auto& device){return device.category==Floppy&&device.backend==backend_->currentData().toString()&&device.controller==controller;});
    if(count>=MaxGwDrives){failure(backend_->currentData().toString()==KryoFlux?MaximumKryoDrivesKey:MaximumDrivesKey);return;}
    for(const auto& row:rows_)if(row->device.controller==controller&&row->device.id.isEmpty()){row->size->setFocus();return;}
    HardwareDevice device;device.category=Floppy;device.backend=backend_->currentData().toString();device.controller=controller;device.address=UsbAddress;
    for(const auto& row:rows_)if(row->device.controller==controller){device.address=row->device.address;device.available=row->device.available;break;}
    if(device.backend==KryoFlux){device.selection=Select0;for(const auto& other:state_.hardware().devices)if(other.backend==KryoFlux&&other.controller==controller&&other.selection==Select0)device.selection=Select1;}
    drafts_.push_back(device);rebuildRows();
}
void FloppySettingsPage::saveReader(const std::shared_ptr<ReaderRow>& row){
    auto device=row->device;device.name=QString::fromStdString(translations_.text(ReaderNumberKey)).replace(Argument,QString::number(row->number));
    device.size=row->size->currentData().toString();device.density=row->density->currentData().toString();device.rpm=row->rpm->currentData().toInt();if(row->selection)device.selection=row->selection->currentData().toString();
    auto settings=state_.hardware();
    if(device.backend==KryoFlux&&std::any_of(settings.devices.cbegin(),settings.devices.cend(),[&](const auto& other){return other.id!=device.id&&other.backend==KryoFlux&&other.controller==device.controller&&other.selection==device.selection;})){failure(KryoDriveInUseKey);return;}
    if(!settings.saveDevice(device)){failure(device.backend==KryoFlux?MaximumKryoDrivesKey:MaximumDrivesKey);return;}
    statusKey_.clear();status_->clear();state_.setHardware(std::move(settings));drafts_.removeIf([&](const auto& draft){return draft.backend==device.backend&&draft.controller==device.controller;});state_.floppyReaderEdits.removeIf([&](const auto& edit){return edit.backend==device.backend&&edit.controller==device.controller&&edit.id==device.id;});rebuildRows();
}
void FloppySettingsPage::forgetReader(const std::shared_ptr<ReaderRow>& row){
    const auto controller=row->device.controller;const auto id=row->device.id;const auto backend=row->device.backend;
    if(id.isEmpty()){drafts_.removeIf([&](const auto& draft){return draft.backend==backend&&draft.controller==controller;});detected_.removeIf([&](const auto& host){return host.backend==backend&&host.id==controller;});state_.floppyReaderEdits.removeIf([&](const auto& edit){return edit.backend==backend&&edit.controller==controller&&edit.id.isEmpty();});rebuildRows();return;}
    const auto count=std::count_if(state_.hardware().devices.cbegin(),state_.hardware().devices.cend(),[&](const auto& device){return device.category==Floppy&&device.backend==backend_->currentData().toString()&&device.controller==controller;});
    confirmation_=std::make_unique<QMessageBox>(QMessageBox::Question,QString::fromStdString(translations_.text(ForgetKey)),
        QString::fromStdString(translations_.text(count==1&&backend==Greaseweazle?ForgetLastKey:ForgetConfirmKey)),QMessageBox::Yes|QMessageBox::No,this);
    confirmation_->setDefaultButton(QMessageBox::No);confirmation_->button(QMessageBox::Yes)->setText(QString::fromStdString(translations_.text(YesKey)));confirmation_->button(QMessageBox::No)->setText(QString::fromStdString(translations_.text(NoKey)));confirmation_->setTextFormat(Qt::PlainText);
    connect(confirmation_.get(),&QMessageBox::finished,this,[this,id,controller,backend](int result){if(result!=QMessageBox::Yes)return;
        auto settings=state_.hardware();settings.forgetDevice(id);state_.setHardware(std::move(settings));
        if(std::none_of(state_.hardware().devices.cbegin(),state_.hardware().devices.cend(),[&](const auto& device){return device.backend==backend&&device.controller==controller;}))detected_.removeIf([&](const auto& host){return host.backend==backend&&host.id==controller;});
        state_.floppyReaderEdits.removeIf([&](const auto& edit){return edit.id==id;});
        statusKey_.clear();status_->clear();rebuildRows();});confirmation_->open();
}
void FloppySettingsPage::scan(){
    rememberReaderEdits();
    if(backend_->currentData().toString()==KryoFlux){
        kryoTools_->synchronizeState();
        try{
            const auto found=Infrastructure::discoverHardware(KryoFlux);detected_.removeIf([](const auto& host){return host.backend==KryoFlux;});detected_+=found;
            auto settings=state_.hardware();settings.applyFloppyDiscovery(KryoFlux,found);
            state_.setHardware(std::move(settings));rebuildRows();statusKey_=found.isEmpty()?NoDevicesKey:QString();refreshTexts();
        }catch(const std::exception& error){failure(error.what());}
        return;
    }
    tools_->synchronizeState();scan_->setEnabled(false);backend_->setEnabled(false);add_->setEnabled(false);statusKey_=ScanningKey;status_->setText(QString::fromStdString(translations_.text(ScanningKey)));
    discovery_->scan(state_.hardware().gwPath,[this](QVector<Infrastructure::HostDevice> found){
        detected_.removeIf([](const auto& host){return host.backend==Greaseweazle;});detected_+=found;auto settings=state_.hardware();
        settings.applyFloppyDiscovery(Greaseweazle,found);
        state_.setHardware(std::move(settings));rebuildRows();scan_->setEnabled(true);backend_->setEnabled(true);add_->setEnabled(true);statusKey_.clear();status_->clear();
    },[this](const char* key){failure(key);});
}
}
