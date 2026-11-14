#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <QObject>
#include <functional>
#include <memory>
class QThread;
namespace RetroMediaLab {
class OpticalProbe final : public QObject {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API explicit OpticalProbe(QObject* = nullptr);
    RETROMEDIALAB_INFRASTRUCTURE_API ~OpticalProbe() override;
    RETROMEDIALAB_INFRASTRUCTURE_API void query(const QString& id,const QString& address,std::function<void(Infrastructure::OpticalCapabilities)>,std::function<void(const char*)>);
    bool isRunning() const noexcept { return active_; }
private:
    bool active_ = false;
    std::unique_ptr<QThread> worker_;
};
}
