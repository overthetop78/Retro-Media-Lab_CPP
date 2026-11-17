#include "ConversionSession.h"
#include "ConversionSessionConstants.h"
#include "MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/GwFormatCapabilities.h>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <chrono>
#include <limits>
#include <utility>
namespace RetroMediaLab::Infrastructure {
namespace C=ConversionSessionConstants;
namespace M=MediaCommandConstants;
namespace E=MediaEngine;
const std::vector<CaptureOptionDefinition>& conversionOptions() {
    static const std::vector<CaptureOptionDefinition> definitions{
        {M::Tracks,M::TracksLabel,M::TracksDefault},{M::OutputTracks,M::TracksLabel,M::TracksDefault},
        {M::AdjustSpeed,M::AdjustSpeedLabel,M::SpeedDefault},{M::Pll,M::PllLabel,M::PllDefault},
        {M::HardSectors,M::HardSectorsLabel,M::DefaultName,true},{M::Reverse,M::ReverseLabel,M::DefaultName,true},
        {M::DiskDefs,M::DiskDefsLabel,M::DefaultName}};
    return definitions;
}
namespace {
template<class T> bool ready(std::future<T>& value) {return value.valid() && value.wait_for(std::chrono::seconds::zero())==std::future_status::ready;}
E::MessageInfo describe(const std::exception& error) {
    if(const auto* media=dynamic_cast<const E::MediaError*>(&error))return {std::string(E::diagnosticKey(media->diagnostic().code)),media->diagnostic().arguments,MediaSessionConstants::MediaDomain};
    if(dynamic_cast<const std::invalid_argument*>(&error))return {error.what(),{},{}};
    return {MediaSessionConstants::UnexpectedError,{error.what()},{}};
}
bool matches(const E::MediaConversionDestination& destination,const E::ConversionOutput& output) {return destination.formatId==output.formatId && destination.extension==output.extension;}
}
ConversionSession::ConversionSession(std::shared_ptr<GreaseweazleTools> tools,QString directory,std::function<LoggingSettings()> settings,std::function<QString(const E::MessageInfo&)> translate)
    :tools_(std::move(tools)),log_(std::move(directory),std::move(settings)),translate_(std::move(translate)) {
    timer_.setInterval(MediaSessionConstants::PollMilliseconds);
    connect(&timer_,&QTimer::timeout,this,[this]{poll();});
    for(const auto& option:conversionOptions())if(!option.flag)draft.values.insert(option.argument,QString::fromUtf8(option.initial));
}
ConversionSession::~ConversionSession() {
    timer_.stop();cancellation_.request_stop();
    if(hostRunning_)tools_->abortAndWait();
    if(opening_.valid())opening_.wait();if(preflight_.valid())preflight_.wait();if(conversion_.valid())conversion_.wait();
}
bool ConversionSession::busy()const{return opening_.valid() || running_;}
void ConversionSession::open(const QString& path,bool internal) {
    if(busy())return;
    error_.reset();draft.source=path;draft.name=QFileInfo(path).completeBaseName();draft.selections.clear();formats_.clear();source_={};openedPath_.clear();sourceInternal_=internal;
    cancellation_=std::stop_source{};const auto token=cancellation_.get_token();
    opening_=std::async(std::launch::async,[path,internal,token] {
        Source value;const auto source=std::filesystem::path(path.toStdU16String());
        if(!QFileInfo(path).isFile())throw std::invalid_argument(C::SourceRequired);
        if(internal) {
            value.document=E::analyzeMediaOpeningAsync(source,{},{},{},token,false).get().document;
            value.direct=E::availableConversionDestinations(value.document);
            value.sequential=E::availableSequentialConversionDestinationsAsync(value.document,{},{},token).get();
            auto destinations=value.direct;destinations.insert(destinations.end(),value.sequential.begin(),value.sequential.end());
            for(const auto& destination:destinations) {
                auto found=std::find_if(value.formats.begin(),value.formats.end(),[&](const auto& format){return format.id==destination.formatId;});
                if(found==value.formats.end()) {
                    E::DiskFormat format;if(const auto* curated=E::findImageFormat(destination.formatId))format=*curated;
                    else {format.id=destination.formatId;format.family=destination.formatId;format.technicalDisplayName=destination.formatId;}
                    // Runtime destinations have already been checked against this document by the engine.
                    // Preserve the C# runtime catalogue's source compatibility instead of the curated filter.
                    format.compatibleSourceExtensions=std::vector<std::string>{source.extension().string()};
                    format.extensions.clear();value.formats.push_back(std::move(format));found=std::prev(value.formats.end());
                }
                if(std::none_of(found->extensions.begin(),found->extensions.end(),[&](const auto& extension){return extension.extension==destination.extension;}))found->extensions.push_back({destination.extension,{},found->extensions.empty(),destination.extension});
            }
        } else value.formats=E::ConversionPlanner{}.outputsForSource(source.extension().string());
        E::checkCancellation(token);return value;
    });timer_.start();publish();
}
std::vector<E::EnabledOption> ConversionSession::options()const {
    std::vector<E::EnabledOption> result;
    for(const auto& option:conversionOptions())if(draft.enabled.contains(option.argument))result.push_back({option.argument,option.flag?std::optional<std::string>{}:draft.values.value(option.argument).toStdString()});
    validateGwOptions(result);
    if(draft.enabled.contains(M::DiskDefs) && !QFileInfo(draft.values.value(M::DiskDefs)).isFile())throw std::invalid_argument(M::MissingDefinition);
    return result;
}
std::vector<E::ConversionOutput> ConversionSession::plan(const ConversionSettings& settings)const {
    if(draft.source!=openedPath_ || !QFileInfo(draft.source).isFile())throw std::invalid_argument(C::SourceRequired);
    if(draft.name.trimmed().isEmpty() || draft.name.contains(QLatin1Char(M::PathSeparator)) || draft.name.contains(QLatin1Char(M::OtherPathSeparator)))throw std::invalid_argument(C::NameRequired);
    options();tokenizeExpertArguments(draft.expert);
    auto result=E::ConversionPlanner(formats_).plan(std::filesystem::path(draft.source.toStdU16String()),draft.name.trimmed().toStdString(),draft.selections,draft.addTags,settings.tagPattern.toStdString());
    if(result.empty())throw std::invalid_argument(C::SelectOutput);
    return result;
}
namespace {
GwToolCommand buildCommand(const ConversionDraft& draft,const HardwareSettings& hardware,const QString& directory,const E::ConversionOutput& target,const std::vector<E::EnabledOption>& enabled) {
    std::vector<std::string> definitions;
    E::appendBuiltInDiskDefinitionArguments(definitions,target.formatId,enabled,std::filesystem::path(directory.toStdU16String()));
    QStringList arguments{M::Convert};for(const auto& value:definitions)arguments.push_back(QString::fromStdString(value));
    if(const auto format=E::gwFormatArgument(target.formatId))arguments<<M::Format<<QString::fromStdString(*format);
    for(const auto& option:enabled){arguments.push_back(QString::fromStdString(option.argument));if(option.value && !option.value->empty())arguments.push_back(QString::fromStdString(*option.value));}
    arguments.append(tokenizeExpertArguments(draft.expert));arguments<<draft.source<<QString::fromStdU16String(target.outputPath.u16string());return {hardware.gwPath,arguments};
}
}
GwToolCommand ConversionSession::command(const E::ConversionOutput& target)const {return buildCommand(draft,hardware_,applicationDirectory_,target,options());}
QString ConversionSession::preview(const HardwareSettings& hardware,const EngineSettings& engines,const ConversionSettings& settings,const QString& directory)const {
    const auto planned=plan(settings);
    const bool internal=engines.engineFor(EngineConstants::Greaseweazle,EngineConstants::Conversion)==EngineConstants::Internal;
    if(internal!=sourceInternal_)throw std::invalid_argument(C::SourceRequired);
    QStringList commands;
    // Preview uses the exact same builder without changing session execution state.
    for(const auto& target:planned) {
        if(internal)commands.push_back(GwToolCommand{C::Application,{C::Encode,C::Codec,QString::fromStdString(target.formatId),draft.source,QString::fromStdU16String(target.outputPath.u16string())}}.displayText());
        else {
            if(!QFileInfo(hardware.gwPath).isFile())throw std::invalid_argument(M::ToolMissing);
            commands.push_back(buildCommand(draft,hardware,directory,target,options()).displayText());
        }
    }
    return commands.join(QString::fromLatin1(C::Separator));
}
void ConversionSession::start(const HardwareSettings& hardware,const EngineSettings& engines,const ConversionSettings& settings,const QString& directory,ConversionConflict conflict) {
    if(busy())return;
    if(!tools_->activeOperation().isEmpty())throw std::invalid_argument(M::InvalidOptions);
    commandText_=preview(hardware,engines,settings,directory);
    auto planned=plan(settings);std::vector<E::ConversionOutput> batch;
    QSet<QString> reserved;for(const auto& item:planned)reserved.insert(QString::fromStdU16String(item.outputPath.u16string()).toCaseFolded());
    const auto sourcePath=std::filesystem::path(draft.source.toStdU16String());
    for(auto& target:planned) {
        auto path=QString::fromStdU16String(target.outputPath.u16string());
        std::error_code equivalenceError;
        if(std::filesystem::equivalent(target.outputPath,sourcePath,equivalenceError))throw std::invalid_argument(C::NameRequired);
        if(QFileInfo::exists(path)) {
            if(conflict==ConversionConflict::Skip)continue;
            if(conflict==ConversionConflict::Number) {
                const QFileInfo file(path);for(int number=1;;++number){
                    path=file.dir().filePath(QString::fromLatin1(C::Numbered).arg(file.completeBaseName()).arg(number).arg(QLatin1Char(C::ExtensionSeparator)+file.suffix()));
                    if(!QFileInfo::exists(path) && !reserved.contains(path.toCaseFolded()))break;
                    if(number==std::numeric_limits<int>::max())throw std::invalid_argument(C::NameRequired);
                }target.outputPath=std::filesystem::path(path.toStdU16String());reserved.insert(path.toCaseFolded());
            }
        }
        batch.push_back(std::move(target));
    }
    if(batch.empty())return;
    batch_=std::move(batch);hardware_=hardware;applicationDirectory_=directory;
    external_=engines.engineFor(EngineConstants::Greaseweazle,EngineConstants::Conversion)!=EngineConstants::Internal;
    messages_.clear();error_.reset();losses_.clear();position_=succeeded_=failed_=0;running_=true;acceptedLosses_=awaitingLosses_=false;cancellation_=std::stop_source{};
    {const std::lock_guard lock(progressMutex_);currentProgress_=0;progressMessage_.reset();}
    outputText_.clear();if(!external_)log_.begin(M::Convert,commandText_);
    try {if(external_)next();else {
        const auto source=source_.document;const auto direct=source_.direct;const auto targets=batch_;const auto token=cancellation_.get_token();
        preflight_=std::async(std::launch::async,[source,direct,targets,token]{
            std::vector<E::MessageInfo> losses;
            for(const auto& target:targets)if(std::none_of(direct.begin(),direct.end(),[&](const auto& destination){return matches(destination,target);})){
                const auto plan=E::planSequentialConversionAsync(source,target.formatId,{},{},token).get();losses.insert(losses.end(),plan.losses.begin(),plan.losses.end());
            }return losses;
        });
    }}catch(...){running_=false;throw;}
    timer_.start();publish();
}
void ConversionSession::acceptLosses(bool accepted){if(!awaitingLosses_)return;awaitingLosses_=false;if(accepted){acceptedLosses_=true;next();}else cancel();publish();}
void ConversionSession::cancel(){cancellation_.request_stop();if(hostRunning_)tools_->stop();if(awaitingLosses_){awaitingLosses_=false;running_=false;}publish();}
void ConversionSession::next() {
    if(cancellation_.stop_requested() || position_>=batch_.size()){running_=false;report({cancellation_.stop_requested()?M::Cancelled:C::Summary,cancellation_.stop_requested()?std::vector<std::string>{}:std::vector<std::string>{std::to_string(succeeded_),std::to_string(failed_)},{}});publish();return;}
    try {
    const auto& target=batch_[position_];
    {const std::lock_guard lock(progressMutex_);currentProgress_=0;}
    report({external_?C::ExternalStart:C::InternalStart,external_?std::vector<std::string>{target.outputPath.filename().string()}:std::vector<std::string>{std::filesystem::path(draft.source.toStdU16String()).filename().string(),target.outputPath.filename().string()},{}});
    if(external_){tools_->executeCommand(M::Convert,command(target));hostRunning_=true;return;}
    const auto token=cancellation_.get_token();
    if(std::any_of(source_.direct.begin(),source_.direct.end(),[&](const auto& destination){return matches(destination,target);})) {
        conversion_=E::convertMediaAsync(E::MediaConversionRequest(source_.document,target.outputPath,target.formatId,{},[this](const E::MediaConversionProgress& progress){const std::lock_guard lock(progressMutex_);currentProgress_=progress.percent;progressMessage_=progress.message;if(progressMessage_->translationDomain.empty())progressMessage_->translationDomain=MediaSessionConstants::MediaDomain;}),token);
    }else conversion_=E::convertSequentialMediaAsync(std::filesystem::path(draft.source.toStdU16String()),target.outputPath,target.formatId,{},{},acceptedLosses_,token);
    }catch(const std::exception& error){error_=describe(error);report(*error_);++failed_;++position_;QTimer::singleShot(0,this,[this]{next();});}
}
void ConversionSession::complete(bool success){if(success)++succeeded_;else ++failed_;++position_;next();}
void ConversionSession::poll() {
    bool processingConversion=false;
    try {
        // Drain worker progress before final messages, so it cannot overwrite the batch summary.
        {std::optional<E::MessageInfo> value;{const std::lock_guard lock(progressMutex_);value=std::exchange(progressMessage_,{});}if(value)report(std::move(*value));}
        if(ready(opening_)){auto value=opening_.get();if(!cancellation_.stop_requested()){source_=std::move(value);formats_=source_.formats;openedPath_=draft.source;}publish();}
        if(ready(preflight_)){losses_=preflight_.get();if(cancellation_.stop_requested())next();else if(!losses_.empty())awaitingLosses_=true;else next();publish();}
        if(ready(conversion_)){
            processingConversion=true;
            const auto result=conversion_.get();for(const auto& message:result.diagnostics)report(message);for(const auto& message:result.losses)report(message);
            if(!cancellation_.stop_requested())report({C::Complete,{batch_[position_].outputPath.filename().string()},{}});complete(!cancellation_.stop_requested());publish();
        }
        if(hostRunning_){const auto& result=tools_->draft(M::Convert);if(output)output(result.lastCommand,result.output);if(tools_->activeOperation()!=M::Convert){hostRunning_=false;complete(result.summaryKey==M::Completed);publish();}}
    }catch(const std::exception& error){if(!cancellation_.stop_requested()){error_=describe(error);report(*error_);}if(processingConversion)complete(false);else running_=false;publish();}
    catch(...){error_=E::MessageInfo{MediaSessionConstants::UnexpectedError,{},{}};running_=false;publish();}
    if(!busy())timer_.stop();
}
double ConversionSession::progress()const {const std::lock_guard lock(progressMutex_);return batch_.empty()?0:(static_cast<double>(position_)*C::Percent+currentProgress_)/static_cast<double>(batch_.size());}
void ConversionSession::report(E::MessageInfo message){
    if(!external_){const auto text=translate_(message);log_.appendLine(text);outputText_+=text+QString::fromLatin1(C::Separator);if(outputText_.size()>C::MaximumOutputLines)outputText_=outputText_.right(C::MaximumOutputLines);if(output)output(commandText_,outputText_);}
    messages_.push_back(std::move(message));publish();
}
void ConversionSession::changed(){publish();}
ExplorerSession::Subscription ConversionSession::subscribe(std::function<void()> callback){auto result=std::make_shared<const std::function<void()>>(std::move(callback));observers_.push_back(result);return result;}
void ConversionSession::publish(){const auto observers=observers_;std::erase_if(observers_,[](const auto& item){return item.expired();});for(const auto& observer:observers)if(const auto callback=observer.lock())(*callback)();}
}
