#include "MigrationSession.h"
#include "MigrationConstants.h"
#include "MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <chrono>
namespace RetroMediaLab::Infrastructure {
using namespace MigrationConstants;
MigrationSession::MigrationSession(){
    const auto targets=MediaEngine::FileSystemMigrationTargetCatalog::all();
    if(!targets.empty())target_=targets.front().formatId;
    timer_.setInterval(MediaSessionConstants::PollMilliseconds);
    connect(&timer_,&QTimer::timeout,this,[this]{poll();});
}
MigrationSession::~MigrationSession(){
    timer_.stop();cancellation_.request_stop();
    if(opening_.valid())opening_.wait();
    if(writing_.valid())writing_.wait();
}
void MigrationSession::captureError(std::string_view key){
    try{throw;}
    catch(const MediaEngine::MediaError& error){message_=MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)),error.diagnostic().arguments,MediaSessionConstants::MediaDomain};}
    catch(const std::exception& error){message_=MediaEngine::MessageInfo{std::string(key),{error.what()},AppDomain};}
    catch(...){message_=MediaEngine::MessageInfo{MediaSessionConstants::UnexpectedError,{},AppDomain};}
}
void MigrationSession::open(const QString& path){
    if(busy()||path.isEmpty())return;
    source_.reset();explored_.reset();report_.reset();message_.reset();accepted_=false;
    sourcePath_=path;outputPath_.clear();cancellation_=std::stop_source{};cancelled_=false;
    try{opening_=MediaEngine::exploreDiskAsync(std::filesystem::path(path.toStdU16String()),{},{},cancellation_.get_token());timer_.start();}
    catch(...){captureError(SourceFailed);}
    notify();
}
void MigrationSession::suggestOutput(){
    if(sourcePath_.isEmpty()||target_.empty())return;
    const auto& definition=MediaEngine::FileSystemMigrationTargetCatalog::get(target_);
    const QFileInfo input(sourcePath_);
    outputPath_=input.dir().filePath(input.completeBaseName()+QString::fromLatin1(MigratedSuffix)+QString::fromUtf8(definition.extension.data(),static_cast<int>(definition.extension.size())));
}
void MigrationSession::selectTarget(std::string target){
    if(busy()||target==target_)return;
    // Reject invalid identifiers before altering the current draft.
    MediaEngine::FileSystemMigrationTargetCatalog::get(target);
    target_=std::move(target);accepted_=false;message_.reset();suggestOutput();validate();notify();
}
void MigrationSession::acceptLosses(bool accepted){if(busy())return;accepted_=accepted;message_.reset();validate();notify();}
void MigrationSession::setOutput(QString path){if(busy())return;outputPath_=std::move(path);message_.reset();notify();}
void MigrationSession::validate(){
    report_.reset();if(!source_||target_.empty())return;
    try{report_=MediaEngine::FileSystemMigrationService::validate(*source_,target_,accepted_);}
    catch(...){captureError(SourceFailed);}
}
bool MigrationSession::canExecute()const{return !busy()&&source_&&report_&&report_->canExecute()&&!outputPath_.isEmpty();}
void MigrationSession::start(bool overwriteConfirmed){
    if(!canExecute())return;
    const QFileInfo input(sourcePath_),output(outputPath_);
    const auto same=input.absoluteFilePath()==output.absoluteFilePath()||(!input.canonicalFilePath().isEmpty()&&input.canonicalFilePath()==output.canonicalFilePath());
    std::error_code error;const auto equivalent=std::filesystem::equivalent(std::filesystem::path(sourcePath_.toStdU16String()),std::filesystem::path(outputPath_.toStdU16String()),error);
    if(same||(!error&&equivalent)){message_=MediaEngine::MessageInfo{SourceDestinationSame,{},AppDomain};notify();return;}
    if(output.exists()&&!overwriteConfirmed)return;
    cancellation_=std::stop_source{};cancelled_=false;message_.reset();
    try{writing_=MediaEngine::FileSystemMigrationService::writeAsync(*source_,std::filesystem::path(outputPath_.toStdU16String()),target_,accepted_,cancellation_.get_token());timer_.start();}
    catch(...){captureError(Failed);}
    notify();
}
void MigrationSession::cancel(){if(!busy())return;cancelled_=true;cancellation_.request_stop();}
void MigrationSession::poll(){
    using namespace std::chrono;
    if(opening_.valid()&&opening_.wait_for(seconds::zero())==std::future_status::ready){
        try{
            auto result=opening_.get();
            if(!cancelled_){
                explored_=std::move(result);
                if(explored_&&explored_->info().volume&&!explored_->info().volume->fileSystemId.empty()){source_=explored_->info().volume;suggestOutput();validate();}
                else message_=MediaEngine::MessageInfo{SourceUnrecognized,{},AppDomain};
            }
        }catch(...){if(!cancelled_)captureError(SourceFailed);}
        notify();
    }
    if(writing_.valid()&&writing_.wait_for(seconds::zero())==std::future_status::ready){
        try{auto result=writing_.get();if(!cancelled_){report_=std::move(result.report);message_=MediaEngine::MessageInfo{Completed,{outputPath_.toStdString()},AppDomain};}}
        catch(...){if(!cancelled_)captureError(Failed);}
        notify();
    }
    if(!busy())timer_.stop();
}
ExplorerSession::Subscription MigrationSession::subscribe(std::function<void()> callback){auto value=std::make_shared<const std::function<void()>>(std::move(callback));observers_.push_back(value);return value;}
void MigrationSession::notify(){const auto observers=observers_;std::erase_if(observers_,[](const auto& observer){return observer.expired();});for(const auto& observer:observers)if(const auto callback=observer.lock())(*callback)();}
}
