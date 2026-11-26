#pragma once
#include <chrono>
#include "CaptureSession.h"
#include "ConversionConflicts.h"
#include <RetroMediaLab/MediaEngine/MediaReading.h>
#include "Conversion/ConversionSettings.h"
#include "Logging/ConsoleLogSession.h"
#include <RetroMediaLab/MediaEngine/ConversionPlanning.h>
#include <RetroMediaLab/MediaEngine/MediaConversion.h>
#include <RetroMediaLab/MediaEngine/SequentialConversion.h>
#include <mutex>
namespace RetroMediaLab::Infrastructure {
RETROMEDIALAB_INFRASTRUCTURE_API const std::vector<CaptureOptionDefinition>& conversionOptions();
struct ConversionDraft {
    QString source,name,expert;
    bool addTags=false,advancedOpen=false;
    QMap<QString,QString> values;
    QSet<QString> enabled;
    std::vector<MediaEngine::ConversionSelection> selections;
    std::optional<MediaEngine::TapePcmProfile> sourcePcm,targetPcm;
};
class RETROMEDIALAB_INFRASTRUCTURE_API ConversionSession final:public QObject {
public:
    ConversionSession(std::shared_ptr<GreaseweazleTools>,QString,std::function<LoggingSettings()>,std::function<QString(const MediaEngine::MessageInfo&)>,std::shared_ptr<OperationProfiles>);
    ~ConversionSession() override;
    ConversionDraft draft;
    const std::vector<OperationProfile>& profiles()const{return profiles_->forOperation(ProfileOperation::Convert);}
    void applyProfile(std::size_t);
    void saveProfile(QString,bool replace=false);
    std::size_t selectedProfile=0;
    static bool requiresPcmInput(const QString&);
    void open(const QString&,bool internal,std::optional<MediaEngine::TapePcmProfile> = {});
    void cancel();
    void changed();
    bool busy()const;
    LoadingProgress loadingProgress()const;
    qint64 elapsedMilliseconds()const;
    bool converting()const noexcept{return running_;}
    bool awaitingLosses()const noexcept{return awaitingLosses_;}
    void acceptLosses(bool);
    const std::vector<MediaEngine::DiskFormat>& formats()const noexcept{return formats_;}
    const std::vector<MediaEngine::MessageInfo>& losses()const noexcept{return losses_;}
    const std::vector<MediaEngine::MessageInfo>& messages()const noexcept{return messages_;}
    const std::optional<MediaEngine::MessageInfo>& error()const noexcept{return error_;}
    std::vector<MediaEngine::ConversionOutput> plan(const ConversionSettings&)const;
    QString preview(const HardwareSettings&,const EngineSettings&,const ConversionSettings&,const QString&)const;
    void start(const HardwareSettings&,const EngineSettings&,const ConversionSettings&,const QString&,const ConversionConflictDecisions&);
    double progress()const;
    ExplorerSession::Subscription subscribe(std::function<void()>);
    std::function<void(const QString&,const QString&)> output;
private:
    struct Source {
        std::shared_ptr<const MediaEngine::MediaDocument> document;
        std::vector<MediaEngine::MediaConversionDestination> direct,sequential;
        std::vector<MediaEngine::DiskFormat> formats;
    };
    void poll();
    void publish();
    std::chrono::steady_clock::time_point started_{};
    void next();
    void complete(bool);
    void report(MediaEngine::MessageInfo);
    void pruneSelections();
    QString previewOutputs(std::span<const MediaEngine::ConversionOutput>,bool,const HardwareSettings&,const QString&)const;
    GwToolCommand command(const MediaEngine::ConversionOutput&)const;
    std::vector<MediaEngine::EnabledOption> options()const;
    QTimer timer_;
    std::stop_source cancellation_;
    std::future<Source> opening_;
    std::future<std::vector<MediaEngine::MessageInfo>> preflight_;
    std::future<MediaEngine::MediaConversionResult> conversion_;
    std::shared_ptr<GreaseweazleTools> tools_;
    std::shared_ptr<OperationProfiles> profiles_;
    OperationProfiles::Subscription profileSubscription_;
    QString profileId_;
    ConsoleLogSession log_;
    std::function<QString(const MediaEngine::MessageInfo&)> translate_;
    QString outputText_;
    Source source_;
    std::vector<MediaEngine::DiskFormat> formats_;
    std::vector<MediaEngine::ConversionOutput> batch_;
    std::vector<MediaEngine::MessageInfo> losses_,messages_;
    std::optional<MediaEngine::MessageInfo> error_;
    HardwareSettings hardware_;
    std::map<std::string,std::string> targetParameters_,sourceParameters_;
    QString applicationDirectory_,commandText_,openedPath_;
    bool sourceInternal_=false,running_=false,external_=false,hostRunning_=false,awaitingLosses_=false,acceptedLosses_=false;
    std::size_t position_=0,succeeded_=0,failed_=0;
    mutable std::mutex progressMutex_;
    double currentProgress_=0;
    std::optional<MediaEngine::MessageInfo> progressMessage_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
};
}
