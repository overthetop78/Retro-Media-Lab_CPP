#pragma once
#include "ExplorerSession.h"
#include <RetroMediaLab/MediaEngine/DiskExploration.h>
#include <RetroMediaLab/MediaEngine/Migration.h>
namespace RetroMediaLab::Infrastructure {
// The operation and its source outlive any theme presentation.
class RETROMEDIALAB_INFRASTRUCTURE_API MigrationSession final:public QObject {
public:
    MigrationSession();
    ~MigrationSession() override;
    void open(const QString&);
    void selectTarget(std::string);
    void acceptLosses(bool);
    void setOutput(QString);
    void start(bool overwriteConfirmed=false);
    void cancel();
    bool busy()const noexcept{return opening_.valid()||writing_.valid();}
    bool writing()const noexcept{return writing_.valid();}
    bool canExecute()const;
    const QString& sourcePath()const noexcept{return sourcePath_;}
    const QString& outputPath()const noexcept{return outputPath_;}
    const std::string& target()const noexcept{return target_;}
    bool lossesAccepted()const noexcept{return accepted_;}
    const std::shared_ptr<const MediaEngine::FileSystemVolumeInfo>& source()const noexcept{return source_;}
    const std::optional<MediaEngine::MigrationValidationReport>& report()const noexcept{return report_;}
    const std::optional<MediaEngine::MessageInfo>& message()const noexcept{return message_;}
    ExplorerSession::Subscription subscribe(std::function<void()>);
private:
    void poll();
    void validate();
    void suggestOutput();
    void notify();
    void captureError(std::string_view);
    QTimer timer_;
    std::stop_source cancellation_;
    std::future<std::shared_ptr<const MediaEngine::ExploredDisk>> opening_;
    std::future<MediaEngine::MigrationResult> writing_;
    std::shared_ptr<const MediaEngine::ExploredDisk> explored_;
    std::shared_ptr<const MediaEngine::FileSystemVolumeInfo> source_;
    std::optional<MediaEngine::MigrationValidationReport> report_;
    std::optional<MediaEngine::MessageInfo> message_;
    QString sourcePath_,outputPath_;
    std::string target_;
    bool accepted_=false,cancelled_=false;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
};
}
