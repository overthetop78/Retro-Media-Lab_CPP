#pragma once
#include "InfrastructureApi.h"
#include "Media/GwProgressTracker.h"
#include "Hardware/HardwareSettings.h"
#include "Logging/LogSettings.h"
#include <QObject>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <functional>
#include <memory>
namespace RetroMediaLab {
enum class GwParameterKind { Required, Optional, Flag };
struct GwToolParameter { const char* id; const char* label; const char* initial; GwParameterKind kind; };
struct GwToolDefinition { const char* id; QVector<GwToolParameter> parameters; };
RETROMEDIALAB_INFRASTRUCTURE_API const QVector<GwToolDefinition>& gwToolDefinitions();
struct GwToolCommand {
    QString executable;
    QStringList arguments;
    RETROMEDIALAB_INFRASTRUCTURE_API QString displayText() const;
};
struct GwToolDraft {
    QMap<QString, QString> values;
    QSet<QString> enabled;
    QString output, summaryKey, lastCommand;
    QStringList summaryArguments;
    QString model, firmware, port;
    Infrastructure::GwTrackProgress progress;
    bool information = false, networkWarning = false, logFailed = false;
};
class GreaseweazleTools final : public QObject {
public:
    using Subscription = std::shared_ptr<const std::function<void()>>;
    RETROMEDIALAB_INFRASTRUCTURE_API GreaseweazleTools(QString logsDirectory, std::function<LoggingSettings()>, std::function<QString(const QString&)> translate = {});
    RETROMEDIALAB_INFRASTRUCTURE_API ~GreaseweazleTools() override;
    RETROMEDIALAB_INFRASTRUCTURE_API static GwToolCommand build(const QString& verb, const QMap<QString,QString>& values, const QSet<QString>& enabled, const QString& executable, const QString& device = {}, const QString& drive = {});
    RETROMEDIALAB_INFRASTRUCTURE_API GwToolCommand command(const QString& verb, const HardwareSettings&, const QString& deviceId) const;
    RETROMEDIALAB_INFRASTRUCTURE_API const GwToolDraft& draft(const QString&) const;
    RETROMEDIALAB_INFRASTRUCTURE_API void setValue(const QString& verb, const QString& field, const QString& value);
    RETROMEDIALAB_INFRASTRUCTURE_API void setEnabled(const QString& verb, const QString& field, bool);
    RETROMEDIALAB_INFRASTRUCTURE_API void execute(const QString& verb, const HardwareSettings&, const QString& deviceId);
    RETROMEDIALAB_INFRASTRUCTURE_API void executeCommand(const QString& operation, const GwToolCommand& command);
    RETROMEDIALAB_INFRASTRUCTURE_API void stop();
    RETROMEDIALAB_INFRASTRUCTURE_API void abortAndWait();
    RETROMEDIALAB_INFRASTRUCTURE_API QString activeOperation() const;
    RETROMEDIALAB_INFRASTRUCTURE_API Subscription subscribe(std::function<void()>);
private:
    void publish();
    void output();
    void finish(int code, bool normal, bool startFailed = false);
    struct Data;
    std::unique_ptr<Data> data_;
};
}
