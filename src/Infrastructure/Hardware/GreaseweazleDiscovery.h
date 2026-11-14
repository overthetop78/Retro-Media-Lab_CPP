#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <QObject>
#include "Logging/LogSettings.h"
#include <functional>
#include <memory>
class QProcess;
class QTimer;
namespace RetroMediaLab {
class ConsoleLogSession;
class GreaseweazleDiscovery final : public QObject {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API explicit GreaseweazleDiscovery(QString,std::function<LoggingSettings()>,QObject* = nullptr);
    RETROMEDIALAB_INFRASTRUCTURE_API ~GreaseweazleDiscovery() override;
    RETROMEDIALAB_INFRASTRUCTURE_API void scan(const QString&,std::function<void(QVector<Infrastructure::HostDevice>)>,std::function<void(const char*)>);
    RETROMEDIALAB_INFRASTRUCTURE_API static bool confirm(Infrastructure::HostDevice&,const QString& output,bool successful);
private:
    void next();
    void finish(bool);
    QString logsDirectory_;
    std::function<LoggingSettings()> logging_;
    std::unique_ptr<QProcess> process_;
    std::unique_ptr<QTimer> timeout_;
    std::unique_ptr<ConsoleLogSession> session_;
    QVector<Infrastructure::HostDevice> pending_,found_;
    Infrastructure::HostDevice current_;
    std::function<void(QVector<Infrastructure::HostDevice>)> done_;
    std::function<void(const char*)> failed_;
    bool active_=false, anyFailure_=false;
};
}
