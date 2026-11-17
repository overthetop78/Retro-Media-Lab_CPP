#include "CaptureSession.h"
#include "Maintenance/GwToolsConstants.h"
#include <ErrorKeys.h>
#include <RetroMediaLab/MediaEngine/GwFormatCapabilities.h>
#include <RetroMediaLab/MediaEngine/ImageFormatDetection.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
namespace C=MediaCommandConstants;
const std::vector<CaptureOptionDefinition>& captureOptions(CaptureMode mode) {
    static const std::vector<CaptureOptionDefinition> read{
        {C::Revs,C::RevsLabel,C::ReadRevsDefault},{C::Retries,C::RetriesLabel,C::ReadRetriesDefault},{C::Tracks,C::TracksLabel,C::TracksDefault},
        {C::SeekRetries,C::SeekRetriesLabel,C::SeekRetriesDefault},{C::FakeIndex,C::FakeIndexLabel,C::SpeedDefault},{C::HardSectors,C::HardSectorsLabel,C::DefaultName,true},
        {C::AdjustSpeed,C::AdjustSpeedLabel,C::SpeedDefault},{C::Pll,C::PllLabel,C::PllDefault},{C::Reverse,C::ReverseLabel,C::DefaultName,true},
        {C::Densel,C::DenselLabel,C::High},{C::Tg43,C::Tg43Label,C::DefaultName,true},{C::DiskDefs,C::DiskDefsLabel,C::DefaultName}};
    static const std::vector<CaptureOptionDefinition> write{
        {C::NoVerify,C::NoVerifyLabel,C::DefaultName,true},{C::EraseEmpty,C::EraseEmptyLabel,C::DefaultName,true},{C::Retries,C::RetriesLabel,C::WriteRetriesDefault},
        {C::Tracks,C::TracksLabel,C::TracksDefault},{C::PreErase,C::PreEraseLabel,C::DefaultName,true},{C::FakeIndex,C::FakeIndexLabel,C::SpeedDefault},
        {C::HardSectors,C::HardSectorsLabel,C::DefaultName,true},{C::Precomp,C::PrecompLabel,C::PrecompDefault},{C::Reverse,C::ReverseLabel,C::DefaultName,true},
        {C::Densel,C::DenselLabel,C::High},{C::Tg43,C::Tg43Label,C::DefaultName,true},{C::DiskDefs,C::DiskDefsLabel,C::DefaultName}};
    return mode!=CaptureMode::Write?read:write;
}
CaptureSession::CaptureSession(CaptureMode kind,std::shared_ptr<GreaseweazleTools> tools,std::shared_ptr<OperationProfiles> profiles):mode(kind),tools_(std::move(tools)),profiles_(std::move(profiles)) {
    for(const auto& option:captureOptions(mode))if(!option.flag)draft.values.insert(option.argument,QString::fromUtf8(option.initial));
    profileId_=this->profiles().front().id;
    profileSubscription_=profiles_->subscribe([this]{const auto& items=this->profiles();const auto found=std::find_if(items.begin(),items.end(),[this](const auto& item){return item.id==profileId_;});selectedProfile=found==items.end()?0:static_cast<std::size_t>(found-items.begin());profileId_=items[selectedProfile].id;publish();});
    subscription_=tools_->subscribe([this]{update();});
}
CaptureSession::~CaptureSession() {subscription_.reset();if(busy()){tools_->abortAndWait();if(mode!=CaptureMode::Write)QFile::remove(activePath_);}}
QString CaptureSession::operation()const{if(mode==CaptureMode::ExplorerRead)return QString::fromLatin1(C::ReadExplorer);return mode!=CaptureMode::Write?QString::fromLatin1(C::Read):QString::fromLatin1(C::Write);}
bool CaptureSession::canStart()const{return tools_->activeOperation().isEmpty();}
bool CaptureSession::busy()const{return tools_->activeOperation()==operation();}
const GwToolDraft& CaptureSession::execution()const{return tools_->draft(operation());}
QString CaptureSession::targetPath()const {
    if(mode==CaptureMode::Write)return draft.source;
    if(draft.directory.trimmed().isEmpty())throw std::invalid_argument(ErrorKeys::InvalidImageDirectory);
    auto name=draft.fileName.trimmed();if(name.isEmpty() || name.contains(QLatin1Char(C::PathSeparator)) || name.contains(QLatin1Char(C::OtherPathSeparator)))throw std::invalid_argument(C::NameRequired);
    if(draft.autoNumber) {qint64 sequence=0;if(!parseCaptureSequence(draft.sequence,draft.alphabetic,sequence))throw std::invalid_argument(C::InvalidOptions);name+=QLatin1Char(C::Space)+captureSequence(sequence,draft.alphabetic,draft.sequenceWidth);}
    const auto extension=draft.raw?QString::fromLatin1(C::ScpExtension):draft.extension;
    if(extension.isEmpty())throw std::invalid_argument(C::TypeRequired);
    return QDir(draft.directory).filePath(name+extension);
}
GwToolCommand CaptureSession::command(const HardwareSettings& hardware,const EngineSettings& engines,const QString& applicationDirectory)const {
    const auto found=std::find_if(hardware.devices.begin(),hardware.devices.end(),[this](const auto& device){return device.id==draft.deviceId && device.category==HardwareConstants::Floppy;});
    if(found==hardware.devices.end() || !found->available)throw std::invalid_argument(C::NoReader);
    if(found->backend!=HardwareConstants::Greaseweazle || engines.engineFor(found->backend,mode==CaptureMode::ExplorerRead?QString::fromLatin1(EngineConstants::Explorer):operation())!=EngineConstants::HostTools)throw std::invalid_argument(C::MissingProvider);
    if(!QFileInfo(hardware.gwPath).isFile())throw std::invalid_argument(C::ToolMissing);
    const auto path=targetPath();
    if(mode==CaptureMode::Write && !QFileInfo(path).isFile())throw std::invalid_argument(C::SourceRequired);
    if(mode==CaptureMode::Write && draft.formatId.isEmpty())throw std::invalid_argument(C::AmbiguousWrite);
    if(mode!=CaptureMode::Write && !draft.raw && draft.formatId.isEmpty())throw std::invalid_argument(C::TypeRequired);
    std::vector<MediaEngine::EnabledOption> options;
    for(const auto& option:captureOptions(mode))if(draft.enabled.contains(option.argument))options.push_back({option.argument,option.flag?std::optional<std::string>{}:draft.values.value(option.argument).toStdString()});
    validateGwOptions(options);
    if(draft.enabled.contains(C::DiskDefs) && !QFileInfo(draft.values.value(C::DiskDefs)).isFile())throw std::invalid_argument(C::MissingDefinition);
    std::vector<std::string> arguments;
    const auto format=mode!=CaptureMode::Write && draft.raw?std::string{}:draft.formatId.toStdString();
    MediaEngine::appendBuiltInDiskDefinitionArguments(arguments,format,options,std::filesystem::path(applicationDirectory.toStdU16String()));
    QStringList values{QString::fromLatin1(mode==CaptureMode::Write?C::Write:C::Read)};for(const auto& value:arguments)values.push_back(QString::fromStdString(value));
    const auto add=[&](const char* key,const QString& value){if(!value.trimmed().isEmpty())values<<QString::fromLatin1(key)<<value;};
    add(C::Device,found->address);add(C::Drive,found->selection);
    if(mode!=CaptureMode::Write) {if(!draft.raw)add(C::Format,draft.formatId);}
    else if(const auto mapped=MediaEngine::gwFormatArgument(format))add(C::Format,QString::fromStdString(*mapped));
    for(const auto& option:options) {values.push_back(QString::fromStdString(option.argument));if(option.value && !option.value->empty())values.push_back(QString::fromStdString(*option.value));}
    values.append(tokenizeExpertArguments(draft.expert));values.push_back(path);
    return {hardware.gwPath,values};
}
void CaptureSession::start(const HardwareSettings& hardware,const EngineSettings& engines,const QString& directory,bool overwrite) {
    const auto built=command(hardware,engines,directory);
    const auto target=targetPath();
    if(mode!=CaptureMode::Write && QFileInfo::exists(target) && !overwrite)throw std::invalid_argument(C::FileExists);
    if(mode!=CaptureMode::Write && !QDir().mkpath(QFileInfo(target).absolutePath()))throw std::invalid_argument(ErrorKeys::InvalidImageDirectory);
    activePath_=target;completed_.clear();cleanupError_.clear();
    tools_->executeCommand(operation(),built);
}
void CaptureSession::stop(){if(busy())tools_->stop();}
void CaptureSession::changed(){publish();}
void CaptureSession::selectSource(const QString& path) {
    if(busy())return;
    draft.source=path;
    const auto result=MediaEngine::ImageFormatDetector{}.detect(std::filesystem::path(path.toStdU16String()));
    draft.formatId=result.format && result.format->supportsPhysicalWrite?QString::fromStdString(result.format->id):QString{};
    publish();
}
void CaptureSession::applyProfile(std::size_t index) {
    namespace P=ProfileConstants;
    if(busy() || index>=profiles().size())return;
    const auto& profile=profiles()[index];
    draft.expert=profile.values.value(P::Expert);draft.enabled.clear();
    for(const auto& option:captureOptions(mode)){
        const auto key=QString::fromLatin1(option.argument).mid(QString::fromLatin1(P::OptionPrefix).size());
        if(profile.enabled.contains(key))draft.enabled.insert(option.argument);
        if(!option.flag && profile.values.contains(key))draft.values.insert(option.argument,profile.values.value(key));
    }
    draft.formatId=profile.values.value(P::Format);
    if(mode!=CaptureMode::Write){draft.raw=profile.system || profile.values.value(P::Result)==P::Raw;draft.extension=profile.values.value(P::Extension,C::ScpExtension);const auto folder=profile.values.value(P::Folder);if(!folder.trimmed().isEmpty())draft.directory=folder;}
    selectedProfile=index;profileId_=profile.id;publish();
}
void CaptureSession::saveProfile(QString name,bool replace) {
    namespace P=ProfileConstants;
    if(busy())return;
    OperationProfile profile;profile.operation=profileOperation();profile.name=std::move(name);profile.values.insert(P::Expert,draft.expert);profile.values.insert(P::Format,draft.formatId);
    if(mode!=CaptureMode::Write){profile.values.insert(P::Result,draft.raw?P::Raw:P::Known);profile.values.insert(P::Extension,draft.extension);profile.values.insert(P::Folder,draft.directory);}
    for(const auto& option:captureOptions(mode)){const auto key=QString::fromLatin1(option.argument).mid(QString::fromLatin1(P::OptionPrefix).size());if(draft.enabled.contains(option.argument))profile.enabled.insert(key);if(!option.flag)profile.values.insert(key,draft.values.value(option.argument));}
    profileId_=profiles_->save(std::move(profile),replace);
    const auto& items=profiles();selectedProfile=static_cast<std::size_t>(std::find_if(items.begin(),items.end(),[this](const auto& item){return item.id==profileId_;})-items.begin());publish();
}
void CaptureSession::nextAvailable() {
    if(busy() || mode==CaptureMode::Write)return;draft.autoNumber=true;
    qint64 sequence=0;if(!parseCaptureSequence(draft.sequence,draft.alphabetic,sequence))sequence=draft.alphabetic?0:1;
    do {draft.sequence=captureSequence(sequence,draft.alphabetic,1);if(!QFileInfo::exists(targetPath()))break;if(sequence==std::numeric_limits<qint64>::max())throw std::invalid_argument(C::InvalidOptions);++sequence;}while(true);
    publish();
}
void CaptureSession::update() {
    const bool running=busy();
    if(running || wasRunning_) {
        const auto& result=execution();if(output)output(result.lastCommand,result.output);
        if(wasRunning_ && !running) {
            if(result.summaryKey==C::Completed) {
                completed_=activePath_;
                if(mode!=CaptureMode::Write && draft.autoNumber) {qint64 sequence=0;if(parseCaptureSequence(draft.sequence,draft.alphabetic,sequence) && sequence<std::numeric_limits<qint64>::max())draft.sequence=captureSequence(sequence+1,draft.alphabetic,1);}
            } else if(mode!=CaptureMode::Write && result.summaryKey==C::Cancelled && QFileInfo::exists(activePath_)) {QFile partial(activePath_);if(!partial.remove())cleanupError_=partial.errorString();}
        }
    }
    wasRunning_=running;publish();
}
ExplorerSession::Subscription CaptureSession::subscribe(std::function<void()> callback) {auto value=std::make_shared<const std::function<void()>>(std::move(callback));observers_.push_back(value);return value;}
void CaptureSession::publish(){const auto observers=observers_;std::erase_if(observers_,[](const auto& item){return item.expired();});for(const auto& observer:observers)if(const auto callback=observer.lock())(*callback)();}
}
