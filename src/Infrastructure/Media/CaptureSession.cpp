#include "CaptureSession.h"
#include "MediaSessionConstants.h"
#include "OpeningProgressConstants.h"
#include <chrono>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
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
    captureInfoTimer_.setInterval(MediaSessionConstants::PollMilliseconds);QObject::connect(&captureInfoTimer_,&QTimer::timeout,&captureInfoTimer_,[this]{pollCaptureInfo();});
    for(const auto& option:captureOptions(mode))if(!option.flag)draft.values.insert(option.argument,QString::fromUtf8(option.initial));
    profileId_=this->profiles().front().id;
    profileSubscription_=profiles_->subscribe([this]{const auto& items=this->profiles();const auto found=std::find_if(items.begin(),items.end(),[this](const auto& item){return item.id==profileId_;});selectedProfile=found==items.end()?0:static_cast<std::size_t>(found-items.begin());profileId_=items[selectedProfile].id;publish();});
    subscription_=tools_->subscribe([this]{update();});
}
CaptureSession::~CaptureSession() {captureInfoTimer_.stop();sourceCancellation_.request_stop();if(sourcePending_.valid())sourcePending_.wait();captureInfoCancellation_.request_stop();if(captureInfoPending_.valid())captureInfoPending_.wait();subscription_.reset();if(tools_->activeOperation()==operation()){tools_->abortAndWait();if(mode!=CaptureMode::Write)QFile::remove(activePath_);}}
QString CaptureSession::operation()const{if(mode==CaptureMode::ExplorerRead)return QString::fromLatin1(C::ReadExplorer);return mode!=CaptureMode::Write?QString::fromLatin1(C::Read):QString::fromLatin1(C::Write);}
bool CaptureSession::canStart()const{return tools_->activeOperation().isEmpty() && !captureInfoPending_.valid() && !sourcePending_.valid();}
bool CaptureSession::busy()const{return sourcePending_.valid()||tools_->activeOperation()==operation();}
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
    if(captureInfoPending_.valid()||sourcePending_.valid())throw std::invalid_argument(GwToolsConstants::BusyKey);
    const auto built=command(hardware,engines,directory);
    const auto target=targetPath();
    if(mode!=CaptureMode::Write && QFileInfo::exists(target) && !overwrite)throw std::invalid_argument(C::FileExists);
    if(mode!=CaptureMode::Write && !QDir().mkpath(QFileInfo(target).absolutePath()))throw std::invalid_argument(ErrorKeys::InvalidImageDirectory);
    activePath_=target;activeRaw_=draft.raw;completed_.clear();cleanupError_.clear();captureInfo_.reset();captureInfoError_.reset();captureInfoCancellation_=std::stop_source{};
    started_=std::chrono::steady_clock::now();tools_->executeCommand(operation(),built);
}
void CaptureSession::stop(){if(sourcePending_.valid())sourceCancellation_.request_stop();else if(tools_->activeOperation()==operation())tools_->stop();if(summarizing())captureInfoCancellation_.request_stop();}
void CaptureSession::changed(){publish();}
void CaptureSession::selectSource(const QString& path) {
    if(busy()||summarizing())return;draft.source=path;draft.formatId.clear();sourceError_.reset();sourceCancellation_=std::stop_source{};const auto token=sourceCancellation_.get_token();
    sourcePending_=std::async(std::launch::async,[path,token]{return MediaEngine::ImageFormatDetector{}.detect(std::filesystem::path(path.toStdU16String()),{},token);});captureInfoTimer_.start();publish();
}
qint64 CaptureSession::elapsedMilliseconds()const{return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started_).count();}
LoadingProgress CaptureSession::loadingProgress()const{
    namespace P=OpeningProgressConstants;LoadingProgress value;value.stage.resourceKey=preparingSource()?P::Reading:summarizing()?P::Display:mode==CaptureMode::Write?P::Write:P::Read;value.detail=QFileInfo(preparingSource()?draft.source:activePath_).fileName().toStdString();
    if(!preparingSource()&&!summarizing()){const auto& progress=execution().progress;if(progress.total&&*progress.total>0){value.indeterminate=false;value.value=std::clamp(P::Maximum*progress.completed/ *progress.total,P::Zero,P::Maximum);}if(progress.cylinder&&progress.head&&progress.total)value.detailMessage=MediaEngine::MessageInfo{P::TrackProgress,{std::to_string(*progress.cylinder),std::to_string(*progress.head),std::to_string(progress.completed),std::to_string(*progress.total)},{}};}
    return value;
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
    const bool running=tools_->activeOperation()==operation();
    if (mode == CaptureMode::Read && running && !wasRunning_) {
        if (!readCaptures_.empty()) readCaptures_.front().archived = true;
        readCaptures_.insert(readCaptures_.begin(),ReadCaptureEntry{activePath_});
        ++readCaptureRevision_;
    }
    if(running || wasRunning_) {
        const auto& result=execution();if(output)output(result.lastCommand,result.output);
        if(wasRunning_ && !running) {
            if (mode == CaptureMode::Read && !readCaptures_.empty() && !readCaptures_.front().completed) {
                if (result.summaryKey == C::Completed && QFileInfo(activePath_).isFile()) readCaptures_.front().completed = true;
                else readCaptures_.erase(readCaptures_.begin());
                ++readCaptureRevision_;
            }
            if(result.summaryKey==C::Completed) {
                completed_=activePath_;
                if(mode==CaptureMode::Read && activeRaw_){try{captureInfoPending_=MediaEngine::readScpCaptureInfoAsync(std::filesystem::path(completed_.toStdU16String()),captureInfoCancellation_.get_token());captureInfoTimer_.start();}catch(const std::exception& error){captureInfoError_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{error.what()},{}};}}
                if(mode!=CaptureMode::Write && draft.autoNumber) {qint64 sequence=0;if(parseCaptureSequence(draft.sequence,draft.alphabetic,sequence) && sequence<std::numeric_limits<qint64>::max())draft.sequence=captureSequence(sequence+1,draft.alphabetic,1);}
            } else if(mode!=CaptureMode::Write && result.summaryKey==C::Cancelled && QFileInfo::exists(activePath_)) {QFile partial(activePath_);if(!partial.remove())cleanupError_=partial.errorString();}
        }
    }
    wasRunning_=running;publish();
}
void CaptureSession::pollCaptureInfo(){
    if(sourcePending_.valid()&&sourcePending_.wait_for(std::chrono::seconds::zero())==std::future_status::ready){try{auto result=sourcePending_.get();if(!sourceCancellation_.stop_requested())draft.formatId=result.format&&result.format->supportsPhysicalWrite?QString::fromStdString(result.format->id):QString{};}catch(const MediaEngine::MediaError& error){if(!sourceCancellation_.stop_requested())sourceError_=MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)),error.diagnostic().arguments,MediaSessionConstants::MediaDomain};}catch(const std::exception& error){if(!sourceCancellation_.stop_requested())sourceError_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{error.what()},{}};}catch(...){if(!sourceCancellation_.stop_requested())sourceError_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{},{}};}publish();}
    if(captureInfoPending_.valid()&&captureInfoPending_.wait_for(std::chrono::seconds::zero())==std::future_status::ready){try{captureInfo_=captureInfoPending_.get();}catch(const MediaEngine::MediaError& error){if(!captureInfoCancellation_.stop_requested())captureInfoError_=MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)),error.diagnostic().arguments,MediaSessionConstants::MediaDomain};}catch(const std::exception& error){if(!captureInfoCancellation_.stop_requested())captureInfoError_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{error.what()},{}};}catch(...){if(!captureInfoCancellation_.stop_requested())captureInfoError_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{},{}};}
        if (!readCaptures_.empty() && readCaptures_.front().path == completed_) {
            readCaptures_.front().info = captureInfo_;
            readCaptures_.front().infoError = captureInfoError_;
            ++readCaptureRevision_;
        }
        publish();
    }
    if(!sourcePending_.valid()&&!captureInfoPending_.valid())captureInfoTimer_.stop();
}
ExplorerSession::Subscription CaptureSession::subscribe(std::function<void()> callback) {auto value=std::make_shared<const std::function<void()>>(std::move(callback));observers_.push_back(value);return value;}
void CaptureSession::publish(){const auto observers=observers_;std::erase_if(observers_,[](const auto& item){return item.expired();});for(const auto& observer:observers)if(const auto callback=observer.lock())(*callback)();}
}
