#pragma once
#include "MediaCommands.h"
#include "MediaCommandConstants.h"
#include "Maintenance/GreaseweazleTools.h"
#include "Engines/EngineSettings.h"
#include "ExplorerSession.h"
#include "Profiles/OperationProfiles.h"
#include <QSet>
#include <RetroMediaLab/MediaEngine/ScpCaptureInfo.h>
namespace RetroMediaLab::Infrastructure {
enum class CaptureMode {Read,Write,ExplorerRead};
struct CaptureOptionDefinition {const char* argument;const char* label;const char* initial;bool flag=false;};
RETROMEDIALAB_INFRASTRUCTURE_API const std::vector<CaptureOptionDefinition>& captureOptions(CaptureMode);
struct CaptureDraft {
    QString source,directory,fileName,formatId,deviceId,expert;
    QString extension=MediaCommandConstants::ScpExtension;
    bool raw=true,autoNumber=false,alphabetic=false,advancedOpen=false;
    int sequenceWidth=1;
    QString sequence=MediaCommandConstants::DefaultSequence;
    QMap<QString,QString> values;
    QSet<QString> enabled;
};
class RETROMEDIALAB_INFRASTRUCTURE_API CaptureSession final {
public:
    CaptureSession(CaptureMode,std::shared_ptr<GreaseweazleTools>,std::shared_ptr<OperationProfiles>);
    ~CaptureSession();
    CaptureDraft draft;
    const CaptureMode mode;
    QString operation() const;
    QString targetPath() const;
    GwToolCommand command(const HardwareSettings&,const EngineSettings&,const QString& applicationDirectory) const;
    void start(const HardwareSettings&,const EngineSettings&,const QString& applicationDirectory,bool overwrite=false);
    void stop();
    void changed();
    void selectSource(const QString&);
    void applyProfile(std::size_t);
    void saveProfile(QString name,bool replace=false);
    const std::vector<OperationProfile>& profiles() const {return profiles_->forOperation(profileOperation());}
    ProfileOperation profileOperation()const noexcept{return mode!=CaptureMode::Write?ProfileOperation::Read:ProfileOperation::Write;}
    std::size_t selectedProfile=0;
    void nextAvailable();
    bool busy() const;
    bool canStart() const;
    const GwToolDraft& execution() const;
    const std::optional<MediaEngine::ScpCaptureInfo>& captureInfo()const noexcept{return captureInfo_;}
    const std::optional<MediaEngine::MessageInfo>& captureInfoError()const noexcept{return captureInfoError_;}
    bool summarizing()const noexcept{return captureInfoPending_.valid();}
    const QString& completedPath() const noexcept {return completed_;}
    const QString& cleanupError() const noexcept {return cleanupError_;}
    const QString& cleanupPath() const noexcept {return activePath_;}
    ExplorerSession::Subscription subscribe(std::function<void()>);
    std::function<void(const QString&,const QString&)> output;
private:
    void update();
    void pollCaptureInfo();
    void publish();
    std::shared_ptr<GreaseweazleTools> tools_;
    GreaseweazleTools::Subscription subscription_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
    bool wasRunning_=false,activeRaw_=false;
    std::optional<MediaEngine::ScpCaptureInfo> captureInfo_;
    std::optional<MediaEngine::MessageInfo> captureInfoError_;
    std::future<MediaEngine::ScpCaptureInfo> captureInfoPending_;
    std::stop_source captureInfoCancellation_;
    QTimer captureInfoTimer_;
    QString activePath_,completed_,cleanupError_;
    std::shared_ptr<OperationProfiles> profiles_;
    OperationProfiles::Subscription profileSubscription_;
    QString profileId_;
};
}
