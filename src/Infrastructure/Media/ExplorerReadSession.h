#pragma once
#include "CaptureSession.h"
class QTemporaryDir;
namespace RetroMediaLab::Infrastructure {
class RETROMEDIALAB_INFRASTRUCTURE_API ExplorerReadSession final {
public:
    ExplorerReadSession(std::shared_ptr<ExplorerSession>,std::shared_ptr<GreaseweazleTools>,std::shared_ptr<OperationProfiles>);
    ~ExplorerReadSession();
    void start(const HardwareSettings&,const EngineSettings&,const QString& applicationDirectory,const QString& deviceId);
    void stop();
    bool busy() const noexcept;
    bool canStart() const {return !busy() && !opening_->busy() && capture_->canStart();}
    CaptureSession& capture() noexcept{return *capture_;}
    ExplorerSession::Subscription subscribe(std::function<void()> callback){return capture_->subscribe(std::move(callback));}
private:
    void update();
    std::shared_ptr<ExplorerSession> opening_;
    std::shared_ptr<CaptureSession> capture_;
    std::shared_ptr<QTemporaryDir> directory_;
    bool capturing_=false,openingCapture_=false;
    ExplorerSession::Subscription captureSubscription_,openingSubscription_;
};
}
