#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <QObject>
#include <functional>
#include <memory>
class QThread;
namespace RetroMediaLab {
class DiskProbe final : public QObject {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API explicit DiskProbe(QObject* = nullptr);
    RETROMEDIALAB_INFRASTRUCTURE_API ~DiskProbe() override;
    RETROMEDIALAB_INFRASTRUCTURE_API void query(const QString& controller,const QString& disk,
        std::function<void(Infrastructure::DiskAccessReport)>,std::function<void(const char*)>);
    bool isRunning() const noexcept { return active_; }
private:
    bool active_ = false;
    std::unique_ptr<QThread> worker_;
};
}
