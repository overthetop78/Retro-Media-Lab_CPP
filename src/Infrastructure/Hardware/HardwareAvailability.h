#pragma once
#include "HardwareSettings.h"
#include "Logging/LogSettings.h"
#include <QObject>
#include <QMap>
#include <QStringList>
#include <functional>
#include <memory>
class QThread;
namespace RetroMediaLab {
struct HardwareAvailabilityResult {
    QMap<QString,QVector<Infrastructure::HostDevice>> groups;
    QVector<Infrastructure::DiskController> controllers;
    QStringList errors;
    RETROMEDIALAB_INFRASTRUCTURE_API void applyTo(HardwareSettings&) const;
};
class HardwareAvailability final : public QObject {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API explicit HardwareAvailability(QObject* = nullptr);
    RETROMEDIALAB_INFRASTRUCTURE_API ~HardwareAvailability() override;
    RETROMEDIALAB_INFRASTRUCTURE_API void refresh(HardwareSettings,QString logsDirectory,LoggingSettings,
        std::function<void(HardwareAvailabilityResult)>);
    RETROMEDIALAB_INFRASTRUCTURE_API static HardwareAvailabilityResult collect(const HardwareSettings&,const QString&,const LoggingSettings&);
private:
    std::unique_ptr<QThread> worker_;
    bool active_=false;
};
}
