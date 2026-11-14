#include "MediaDeviceSettingsPage.h"
#include <HardwareDiscoveryConstants.h>
#include "../ThemeIcons.h"
#include "Shared/Controls/Card.h"
#include "Shared/Controls/StatusIndicator.h"
#include "Logging/DiagnosticLog.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include "Hardware/OpticalProbe.h"
#include "Hardware/DiskProbe.h"
#include <QSignalBlocker>
#include <algorithm>
#include <utility>
namespace RetroMediaLab {
using namespace HardwareConstants;
MediaDeviceSettingsPage::MediaDeviceSettingsPage(const HardwareCategory& category,ThemeService& themes,Translations::TranslationService& translations,WorkspaceState& state,QWidget* parent)
    :QWidget(parent),category_(category.id),themes_(themes),translations_(translations),state_(state){
    setObjectName(category_);auto* outer=new QVBoxLayout(this);outer->setContentsMargins(Padding,Padding,Padding,Padding);outer->setSpacing(RowGap);
    if(category_==Disks){
        auto* row=new QHBoxLayout;diskInterfaceLabel_=new QLabel(this);diskControllers_=new QComboBox(this);diskControllers_->setObjectName(DiskInterfaceSelection);diskInterfaceLabel_->setBuddy(diskControllers_);row->addWidget(diskInterfaceLabel_);row->addWidget(diskControllers_,1);outer->addLayout(row);
        connect(diskControllers_,&QComboBox::currentIndexChanged,this,[this]{state_.diskControllerSelection=diskControllers_->currentData().toString();refreshDiskChoices();});
    }
    auto* actions=new QHBoxLayout;scan_=new QPushButton(this);scan_->setObjectName(MediaScan);scan_->setAutoDefault(false);
    choices_=new QComboBox(this);choices_->setObjectName(DeviceSelection);choices_->setMinimumWidth(ComboWidth);
    add_=new QPushButton(this);add_->setObjectName(MediaAdd);add_->setAutoDefault(false);test_=new QPushButton(this);test_->setObjectName(category_==Disks?TestDisk:TestOptical);test_->setAutoDefault(false);test_->setVisible(category_==Optical||category_==Disks);test_->setEnabled(false);actions->addWidget(scan_);actions->addWidget(choices_,1);actions->addWidget(test_);actions->addWidget(add_);outer->addLayout(actions);
    auto* scroll=new QScrollArea(this);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* host=new QWidget(scroll);body_=new QVBoxLayout(host);body_->setContentsMargins(0,0,0,0);body_->setSizeConstraint(QLayout::SetMinAndMaxSize);scroll->setWidget(host);outer->addWidget(scroll,1);
    explanation_=new QLabel(this);explanation_->setWordWrap(true);outer->addWidget(explanation_);status_=new QLabel(this);status_->setObjectName(Status);status_->setWordWrap(true);status_->setTextFormat(Qt::PlainText);outer->addWidget(status_);
    connect(scan_,&QPushButton::clicked,this,[this]{scan();});connect(add_,&QPushButton::clicked,this,[this]{addDevice();});
    connect(test_,&QPushButton::clicked,this,[this]{if(const auto* host=selectedDevice()){if(category_==Disks)testDisk(host->parentId,host->id);else testOptical(host->id,host->address);}});
    connect(choices_,&QComboBox::currentIndexChanged,this,[this]{if(category_==Disks){test_->setEnabled(selectedDevice()&&(!diskProbe_||!diskProbe_->isRunning()));rebuild();}else if(category_==Optical)rebuild();});
    subscription_=translations.subscribe([this]{refreshTexts();rebuild();});refreshTexts();rebuild();
    if(category_==Disks){
        if(state_.diskControllerSelection.isEmpty())for(const auto& device:state_.hardware().devices)if(device.category==Disks){state_.diskControllerSelection=device.controller;break;}
        if(!state_.diskControllerSelection.isEmpty())scan();
    }
}
MediaDeviceSettingsPage::~MediaDeviceSettingsPage()=default;
void MediaDeviceSettingsPage::scan(){
    try{
        detected_=Infrastructure::discoverHardware(category_);if(category_==Disks)controllerInventory_=Infrastructure::diskControllers();auto settings=state_.hardware();
        settings.applyMediaDiscovery(category_,detected_,controllerInventory_);
        state_.setHardware(std::move(settings));{const QSignalBlocker blocked(choices_);choices_->clear();for(const auto& host:detected_)choices_->addItem(deviceLabel(host),host.id);}
        if(category_==Disks){
            const QSignalBlocker blocked(diskControllers_);diskControllers_->clear();
            for(const auto& controller:controllerInventory_){QString label=controller.name;if(!controller.connection.isEmpty())label+=QString::fromUtf8(DeviceLabelFormat).arg(QString(),controller.connection);if(!controller.location.isEmpty())label+=QString::fromUtf8(DeviceLabelFormat).arg(QString(),controller.location);diskControllers_->addItem(label,controller.id);diskControllers_->setItemData(diskControllers_->count()-1,controller.id,Qt::ToolTipRole);}
            const auto previous=diskControllers_->findData(state_.diskControllerSelection);diskControllers_->setCurrentIndex(previous>=0?previous:diskControllers_->count()?0:-1);state_.diskControllerSelection=diskControllers_->currentData().toString();refreshDiskChoices();
        }
        test_->setEnabled(category_==Optical&&!detected_.isEmpty()&&(!probe_||!probe_->isRunning()));
        choices_->setEnabled(choices_->count()>0);add_->setEnabled(category_==Disks?diskControllers_->count()>0:!detected_.isEmpty());statusKey_=(category_==Disks?controllerInventory_.isEmpty():detected_.isEmpty())?QString(NoSupportedDevicesKey):QString();status_->setText(statusKey_.isEmpty()?QString():QString::fromStdString(translations_.text(statusKey_.toStdString())));rebuild();if(category_==Disks)refreshDiskChoices();
    }catch(const std::exception& error){statusKey_=QString::fromUtf8(error.what());status_->setText(QString::fromStdString(translations_.text(error.what())));
        writeDiagnostic(state_.logsDirectory,DiagnosticSeverity::Error,status_->text(),DiagnosticContext,QString::fromStdString(translations_.language()),DiagnosticProcess);}
}
void MediaDeviceSettingsPage::addDevice(){
    if(category_==Disks){
        const auto id=diskControllers_->currentData().toString();const auto controller=std::find_if(controllerInventory_.cbegin(),controllerInventory_.cend(),[&](const auto& item){return item.id==id;});if(controller==controllerInventory_.cend())return;
        auto settings=state_.hardware();if(settings.saveDiskInterface(*controller))state_.setHardware(std::move(settings));rebuild();refreshDiskChoices();return;
    }
    const auto* selected=selectedDevice();if(!selected)return;
    auto settings=state_.hardware();if(settings.saveDetectedDevice(category_,*selected))state_.setHardware(std::move(settings));rebuild();
}
void MediaDeviceSettingsPage::rebuild(){
    if(cards_){body_->removeWidget(cards_);cards_->hide();cards_->deleteLater();}
    cards_=new QWidget(this);cards_->setObjectName(MediaRows);auto* layout=new QVBoxLayout(cards_);layout->setContentsMargins(0,0,0,0);layout->setSpacing(RowGap);layout->setSizeConstraint(QLayout::SetMinAndMaxSize);body_->addWidget(cards_);
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    const auto withError=[&](const QString& id,QVector<CapabilityRow> rows){const auto error=state_.hardwareProbeErrors.value(id);if(!error.isEmpty())rows.push_back({text(category_==Disks?TestInterfaceKey:TestDriveKey),CapabilityState::Fail,text(SourceTestKey),QString::fromStdString(translations_.text(error.toStdString()))});return rows;};
    auto devices=state_.hardware().devices;
    if(category_==Disks){const auto selected=diskControllers_->currentData().toString();std::stable_sort(devices.begin(),devices.end(),[&](const auto& left,const auto& right){return left.controller==selected&&right.controller!=selected;});}
    for(const auto& device:devices)if(device.category==category_){
        auto* card=new Card(themes_.data(),cards_);auto* grid=new QGridLayout(card);grid->setContentsMargins(RowPadding,RowPadding,RowPadding,RowPadding);grid->setSpacing(RowGap);grid->setSizeConstraint(QLayout::SetMinAndMaxSize);
        auto* header=new QWidget(card);auto* headerLayout=new QHBoxLayout(header);headerLayout->setContentsMargins(0,0,0,0);grid->addWidget(header,0,0,1,EditorColumns);
        auto* name=new QLabel(device.name,header);name->setWordWrap(true);name->setTextFormat(Qt::PlainText);name->setToolTip(device.address);auto font=name->font();font.setBold(true);name->setFont(font);headerLayout->addWidget(name,1);
        auto* available=new StatusIndicator(header);available->setStatus(text(device.available?AvailableStateKey:UnavailableStateKey),themes_.data()->color(device.available?SuccessBrush:MutedBrush));headerLayout->addWidget(available);
        auto* remove=new QPushButton(header);remove->setAutoDefault(false);remove->setObjectName(ForgetButton);remove->setFixedWidth(IconButtonWidth);remove->setIcon(themeIcon(TrashIcon,themes_.data()->color(ErrorBrush)));remove->setToolTip(text(ForgetKey));remove->setAccessibleName(text(ForgetKey));headerLayout->addWidget(remove);
        connect(remove,&QPushButton::clicked,this,[this,id=device.id]{forget(id);});
        if(category_==Optical){
            auto* test=new QPushButton(text(TestDriveKey),card);test->setObjectName(TestOptical);test->setAutoDefault(false);test->setEnabled(!probe_||!probe_->isRunning());grid->addWidget(test,1,0,1,EditorColumns,Qt::AlignLeft);
            connect(test,&QPushButton::clicked,this,[this,id=device.controller,address=device.address]{testOptical(id,address);});
            const auto found=state_.opticalCapabilities.constFind(device.controller);
            if(found!=state_.opticalCapabilities.cend())grid->addWidget(new HardwareCapabilitiesTable(withError(device.controller,opticalRows(*found)),themes_.data(),translations_,card),OpticalDetailsRow,0,1,EditorColumns);
            else if(state_.hardwareProbeErrors.contains(device.controller))grid->addWidget(new HardwareCapabilitiesTable(withError(device.controller,{}),themes_.data(),translations_,card),OpticalDetailsRow,0,1,EditorColumns);
            else grid->addWidget(new QLabel(text(UnverifiedKey),card),OpticalDetailsRow,0,1,EditorColumns);
        }
        if(category_==Disks){
            const auto* attached=selectedDevice();if(attached&&attached->parentId!=device.controller&&attached->id!=device.controller)attached=nullptr;
            auto* test=new QPushButton(text(TestInterfaceKey),card);test->setObjectName(TestDisk);test->setAutoDefault(false);test->setEnabled(attached&&(!diskProbe_||!diskProbe_->isRunning()));grid->addWidget(test,1,0,1,EditorColumns,Qt::AlignLeft);
            if(attached)connect(test,&QPushButton::clicked,this,[this,id=device.controller,disk=attached->id]{testDisk(id,disk);});
            const auto found=state_.diskInterfaceReports.constFind(device.controller);
            if(found!=state_.diskInterfaceReports.cend())grid->addWidget(new HardwareCapabilitiesTable(withError(device.controller,diskRows(*found)),themes_.data(),translations_,card),OpticalDetailsRow,0,1,EditorColumns);
            else if(state_.hardwareProbeErrors.contains(device.controller))grid->addWidget(new HardwareCapabilitiesTable(withError(device.controller,{}),themes_.data(),translations_,card),OpticalDetailsRow,0,1,EditorColumns);
            else grid->addWidget(new QLabel(text(ProbeNotTestedKey),card),OpticalDetailsRow,0,1,EditorColumns);
        }
        if(category_==Audio){
            auto* audio=new QWidget(card);auto* options=new QGridLayout(audio);options->setContentsMargins(0,0,0,0);
            auto* rate=new QComboBox(audio);rate->setObjectName(RateInput);for(const auto value:{DefaultRate,Rate48,Rate96})rate->addItem(QString::number(value)+HzSuffix,value);
            if(rate->findData(device.sampleRate)<0)rate->addItem(QString::number(device.sampleRate)+HzSuffix,device.sampleRate);rate->setCurrentIndex(rate->findData(device.sampleRate));
            auto* channels=new QSpinBox(audio);channels->setObjectName(ChannelsInput);channels->setRange(1,ChannelMax);channels->setValue(device.channels);
            auto* channel=new QSpinBox(audio);channel->setObjectName(ChannelInput);channel->setRange(0,device.channels-1);channel->setValue(device.channel);
            options->addWidget(new QLabel(text(SampleRateKey),audio),0,0);options->addWidget(rate,0,1);
            options->addWidget(new QLabel(text(ChannelsKey),audio),0,AudioSecondLabelColumn);options->addWidget(channels,0,AudioSecondInputColumn);
            options->addWidget(new QLabel(text(ChannelKey),audio),1,0);options->addWidget(channel,1,1);
            auto* save=new QPushButton(audio);save->setObjectName(SaveButton);save->setAutoDefault(false);save->setFixedWidth(IconButtonWidth);save->setIcon(themeIcon(SaveIcon,themes_.data()->color(MutedBrush)));save->setToolTip(text(SaveKey));save->setAccessibleName(text(SaveKey));options->addWidget(save,1,AudioSecondInputColumn,Qt::AlignRight);
            connect(channels,&QSpinBox::valueChanged,channel,[channel](int count){channel->setMaximum(count-1);});
            connect(save,&QPushButton::clicked,this,[this,device=HardwareDevice(device),rate,channels,channel]()mutable{device.sampleRate=rate->currentData().toInt();device.channels=channels->value();device.channel=channel->value();auto settings=state_.hardware();if(settings.saveDevice(device))state_.setHardware(std::move(settings));});
            grid->addWidget(audio,1,0,1,EditorColumns);
        }
        layout->addWidget(card);
    }
    const auto selected=category_==Disks?diskControllers_->currentData().toString():choices_->currentData().toString();
    const bool configured=std::any_of(devices.cbegin(),devices.cend(),[&](const auto& item){return item.category==category_&&item.controller==selected;});
    const bool hasOptical=category_==Optical&&state_.opticalCapabilities.contains(selected);
    const bool hasDisk=category_==Disks&&state_.diskInterfaceReports.contains(selected);
    const bool hasError=state_.hardwareProbeErrors.contains(selected);
    if(!configured&&(hasOptical||hasDisk||hasError)){
        auto* card=new Card(themes_.data(),cards_);auto* content=new QVBoxLayout(card);content->setContentsMargins(RowPadding,RowPadding,RowPadding,RowPadding);content->setSizeConstraint(QLayout::SetMinAndMaxSize);content->setAlignment(Qt::AlignTop);
        const auto rows=withError(selected,hasOptical?opticalRows(state_.opticalCapabilities.value(selected)):hasDisk?diskRows(state_.diskInterfaceReports.value(selected)):QVector<CapabilityRow>{});
        content->addWidget(new HardwareCapabilitiesTable(rows,themes_.data(),translations_,card));layout->addWidget(card);
    }
    if((hasOptical||hasDisk||hasError)&&statusKey_.isEmpty())status_->clear();
    layout->addStretch();add_->setEnabled(category_==Disks?diskControllers_->count()>0:!detected_.isEmpty());choices_->setEnabled(choices_->count()>0);
    if(category_==Optical&&statusKey_.isEmpty()&&std::any_of(state_.hardware().devices.cbegin(),state_.hardware().devices.cend(),[&](const auto& device){return device.controller==choices_->currentData().toString();}))status_->clear();
    if(category_==Disks&&statusKey_.isEmpty()&&std::any_of(state_.hardware().devices.cbegin(),state_.hardware().devices.cend(),[&](const auto& device){return device.controller==diskControllers_->currentData().toString();}))status_->clear();
}
void MediaDeviceSettingsPage::forget(const QString& id){
    confirmation_=std::make_unique<QMessageBox>(QMessageBox::Question,QString::fromStdString(translations_.text(ForgetKey)),QString::fromStdString(translations_.text(ForgetConfirmKey)),QMessageBox::Yes|QMessageBox::No,this);confirmation_->setDefaultButton(QMessageBox::No);confirmation_->button(QMessageBox::Yes)->setText(QString::fromStdString(translations_.text(YesKey)));confirmation_->button(QMessageBox::No)->setText(QString::fromStdString(translations_.text(NoKey)));confirmation_->setTextFormat(Qt::PlainText);
    connect(confirmation_.get(),&QMessageBox::finished,this,[this,id](int answer){if(answer!=QMessageBox::Yes)return;auto settings=state_.hardware();settings.forgetDevice(id);state_.setHardware(std::move(settings));rebuild();});confirmation_->open();
}
QString MediaDeviceSettingsPage::deviceLabel(const Infrastructure::HostDevice& host) const{
    if(category_!=Audio)return host.name;
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    const auto kind=host.inputKind==Infrastructure::AudioInputKind::Line?AudioLineKey:host.inputKind==Infrastructure::AudioInputKind::Microphone?AudioMicrophoneKey:host.inputKind==Infrastructure::AudioInputKind::Digital?AudioDigitalKey:AudioOtherKey;
    auto label=QString::fromUtf8(DeviceLabelFormat).arg(text(kind),host.name);
    if(!host.available)label=QString::fromUtf8(DeviceLabelFormat).arg(label,text(host.state==Infrastructure::HostDeviceState::Disabled?AudioDisabledKey:UnavailableStateKey));
    return label;
}
void MediaDeviceSettingsPage::testOptical(const QString& id,const QString& address){
    if(!probe_)probe_=std::make_unique<OpticalProbe>();
    if(probe_->isRunning())return;
    state_.hardwareProbeErrors.remove(id);statusKey_=TestingDriveKey;status_->setText(QString::fromStdString(translations_.text(TestingDriveKey)));test_->setEnabled(false);
    probe_->query(id,address,[this,id](const Infrastructure::OpticalCapabilities& result){
        state_.opticalCapabilities.insert(id,result);statusKey_.clear();status_->clear();test_->setEnabled(!detected_.isEmpty());rebuild();
    },[this,id](const char* key){state_.hardwareProbeErrors.insert(id,QString::fromUtf8(key));statusKey_.clear();status_->clear();test_->setEnabled(!detected_.isEmpty());rebuild();});
    rebuild();
}
const Infrastructure::HostDevice* MediaDeviceSettingsPage::selectedDevice() const{
    const auto id=choices_->currentData().toString();const auto found=std::find_if(detected_.cbegin(),detected_.cend(),[&](const auto& host){return host.id==id;});return found==detected_.cend()?nullptr:&*found;
}
void MediaDeviceSettingsPage::refreshDiskChoices(){
    if(!diskControllers_)return;
    const auto previous=choices_->currentData().toString();const QSignalBlocker blocked(choices_);choices_->clear();
    const auto id=diskControllers_->currentData().toString();for(const auto& host:detected_)if(host.parentId==id)choices_->addItem(QString::fromStdString(translations_.text(ConnectionNumberKey)).replace(Argument,QString::number(choices_->count()+1)),host.id);
    const auto index=choices_->findData(previous);choices_->setCurrentIndex(index>=0?index:choices_->count()?0:-1);choices_->setEnabled(choices_->count()>0);
    test_->setEnabled(choices_->count()>0&&(!diskProbe_||!diskProbe_->isRunning()));add_->setEnabled(diskControllers_->count()>0);
    explanation_->setText(QString::fromStdString(translations_.text(CapabilityHelpKey))+(choices_->count()?QString():LineBreak+QString::fromStdString(translations_.text(DiskConnectKey))));
    rebuild();
}
QVector<CapabilityRow> MediaDeviceSettingsPage::opticalRows(const Infrastructure::OpticalCapabilities& report) const{
    using namespace HardwareDiscoveryConstants;
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    QVector<CapabilityRow> rows;
    const auto announced=[&](const char* key,bool value,bool known=true){rows.push_back({text(key),known?(value?CapabilityState::Pass:CapabilityState::Fail):CapabilityState::Unknown,text(SourceDeviceKey),known?text(value?SupportedReportedKey:NotReportedKey):text(ProbeNotTestedKey)});};
    announced(OpticalCdKey,report.cdRead);announced(OpticalDvdKey,report.dvdRead);announced(OpticalBdKey,report.bdRead);
    announced(OpticalC2Key,report.c2,report.cdDetailsKnown);announced(OpticalCdTextKey,report.cdText,report.cdDetailsKnown);
    const std::pair<int,const char*> features[]={{MmcRandomRead,OpticalRandomReadKey},{MmcMultiRead,OpticalMultiReadKey},{MmcStreaming,OpticalStreamingKey},{MmcRandomWrite,OpticalRandomWriteKey},{MmcIncrementalWrite,OpticalIncrementalWriteKey},{MmcCdTrackWrite,OpticalCdTrackWriteKey},{MmcCdMastering,OpticalCdMasteringKey},{MmcDvdWrite,OpticalDvdWriteKey},{MmcBdWrite,OpticalBdWriteKey}};
    for(const auto& [code,key]:features)announced(key,report.features.contains(code));
    for(const auto profile:report.profiles){
        const auto code=QString::fromUtf8(HexFormat).arg(profile,HexWidth,HexBase,QChar::fromLatin1(HexPadding));QString label=code;
        for(const auto& entry:MmcProfileLabels)if(entry.code==profile){label=text(entry.key);break;}
        rows.push_back({text(MmcProfileKey).replace(Argument,label),CapabilityState::Pass,text(SourceDeviceKey),code});
    }
    rows.push_back({text(OpticalExtractionKey),CapabilityState::Unknown,text(SourceTestKey),text(ProbeNotTestedKey)});
    return rows;
}
QVector<CapabilityRow> MediaDeviceSettingsPage::diskRows(const Infrastructure::DiskAccessReport& report) const{
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};QVector<CapabilityRow> rows,information;
    const auto info=[&](const char* key,QString value,const char* source=SourceDriverKey){information.push_back({text(key),value.isEmpty()?CapabilityState::Unknown:CapabilityState::Information,text(source),value.isEmpty()?text(UnknownKey):value});};
    info(ControllerNameKey,report.controllerName,SourceConnectionKey);info(ControllerDriverKey,report.controllerDriver);info(ControllerLocationKey,report.controllerLocation,SourceConnectionKey);info(DiskConnectionKey,report.connection,SourceConnectionKey);
    info(BusVersionKey,report.busMajorVersion||report.busMinorVersion?QString::fromUtf8(VersionFormat).arg(report.busMajorVersion).arg(report.busMinorVersion):QString());
    info(DiskTransferKey,report.adapterPropertiesKnown?QString::number(report.maximumTransferBytes)+ByteSuffix:QString());info(PhysicalPagesKey,report.adapterPropertiesKnown?QString::number(report.maximumPhysicalPages):QString());info(AlignmentMaskKey,report.adapterPropertiesKnown?QString::number(report.alignmentMask):QString());
    const auto announced=[&](const char* key,bool value){rows.push_back({text(key),!report.adapterPropertiesKnown?CapabilityState::Unknown:value?CapabilityState::Pass:CapabilityState::Fail,text(SourceDriverKey),report.adapterPropertiesKnown?text(value?SupportedReportedKey:NotReportedKey):text(ProbeNotTestedKey)});};
    const auto nativeDetails=[&](quint32 error,quint8 status){return text(CommandDetailsKey).replace(Argument,QString::number(error)).replace(SecondArgument,QString::number(status));};
    info(ScsiAddressKey,report.addressKnown?text(ScsiAddressFormatKey).replace(Argument,QString::number(report.portNumber)).replace(SecondArgument,QString::number(report.pathId)).replace(ThirdArgument,QString::number(report.targetId)).replace(FourthArgument,QString::number(report.lun)):QString(),SourceConnectionKey);
    const auto rawState=report.rawAccessVerified?CapabilityState::Pass:report.readState==Infrastructure::DiskReadState::Failed?CapabilityState::Fail:CapabilityState::Unknown;
    rows.push_back({text(RawAccessKey),rawState,text(SourceTestKey),report.rawAccessVerified?text(DiskReadOkKey):report.readState==Infrastructure::DiskReadState::AccessDenied?text(ProbeAccessDeniedKey):report.nativeError?nativeDetails(report.nativeError,0):text(ProbeNotTestedKey)});
    const auto command=[&](const char* key,Infrastructure::StorageCommandState state,quint32 error,quint8 status,QString transport={}){
        using State=Infrastructure::StorageCommandState;
        const auto outcome=state==State::Verified?CapabilityState::Pass:state==State::Rejected||state==State::Failed?CapabilityState::Fail:CapabilityState::Unknown;
        auto detail=state==State::Verified?text(ProtocolAcceptedKey):state==State::AccessDenied?text(ProbeAccessDeniedKey):state==State::Rejected?text(ProbeRejectedKey):state==State::Failed?text(ProbeFailedKey):text(ProbeNotTestedKey);
        if(!transport.isEmpty())detail+=Space+transport;if(error||status)detail+=LineBreak+nativeDetails(error,status);
        rows.push_back({text(key),outcome,text(SourceTestKey),detail});
    };
    command(DiskScsiInquiryKey,report.commands.inquiryState,report.commands.inquiryError,report.commands.inquiryStatus);
    command(DiskAtaIdentifyKey,report.commands.ataIdentifyState,report.commands.ataError,report.commands.ataStatus,report.commands.transport);
    announced(DiskQueueKey,report.commandQueueing);announced(PioModeKey,report.adapterUsesPio);announced(AcceleratedTransferKey,report.acceleratedTransfer);
    if(report.elevation!=Infrastructure::DiskElevationState::NotRequired){const auto complete=report.elevation==Infrastructure::DiskElevationState::Completed;const auto cancelled=report.elevation==Infrastructure::DiskElevationState::Cancelled;rows.push_back({text(AdministratorAccessKey),complete?CapabilityState::Pass:cancelled?CapabilityState::Unknown:CapabilityState::Fail,text(SourceTestKey),text(complete?ProbeVerifiedKey:cancelled?ProbeCancelledKey:ProbeElevationFailedKey)});}
    rows+=information;return rows;
}
void MediaDeviceSettingsPage::testDisk(const QString& controller,const QString& disk){
    if(!diskProbe_)diskProbe_=std::make_unique<DiskProbe>();if(diskProbe_->isRunning())return;
    state_.hardwareProbeErrors.remove(controller);statusKey_=TestingDriveKey;status_->setText(QString::fromStdString(translations_.text(TestingDriveKey)));test_->setEnabled(false);
    diskProbe_->query(controller,disk,[this,controller](const Infrastructure::DiskAccessReport& result){state_.diskInterfaceReports.insert(controller,result);statusKey_.clear();status_->clear();refreshDiskChoices();},
        [this,controller](const char* key){state_.hardwareProbeErrors.insert(controller,QString::fromUtf8(key));statusKey_.clear();status_->clear();refreshDiskChoices();});rebuild();
}
void MediaDeviceSettingsPage::refreshTexts(){
    const auto text=[this](const char* key){return QString::fromStdString(translations_.text(key));};
    scan_->setText(text(ScanKey));add_->setText(text(AddDriveKey));test_->setText(text(category_==Disks?TestInterfaceKey:TestDriveKey));
    if(diskInterfaceLabel_)diskInterfaceLabel_->setText(text(DiskInterfaceKey));
    choices_->setAccessibleName(text(category_==Disks?ConnectionToTestKey:TitleKey));
    explanation_->setText(text(category_==Audio?AudioHelpKey:(category_==Disks||category_==Optical)?CapabilityHelpKey:PersistenceKey));
    for(int index=0;index<choices_->count();++index){const auto id=choices_->itemData(index).toString();const auto host=std::find_if(detected_.cbegin(),detected_.cend(),[&](const auto& item){return item.id==id;});if(host!=detected_.cend())choices_->setItemText(index,category_==Disks?text(ConnectionNumberKey).replace(Argument,QString::number(index+1)):deviceLabel(*host));}
    if(!statusKey_.isEmpty())status_->setText(QString::fromStdString(translations_.text(statusKey_.toStdString())));
}
}
