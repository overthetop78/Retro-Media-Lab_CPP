#pragma once
#include "LogSettings.h"
#include <QDateTime>
#include <functional>
#include <mutex>
namespace RetroMediaLab {
class ConsoleLogSession final {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API ConsoleLogSession(QString directory, std::function<LoggingSettings()> settings,
        std::function<QDateTime()> clock = [] { return QDateTime::currentDateTime(); });
    RETROMEDIALAB_INFRASTRUCTURE_API bool begin(const QString& action,const QString& command) noexcept;
    RETROMEDIALAB_INFRASTRUCTURE_API bool appendLine(const QString&) noexcept;
    RETROMEDIALAB_INFRASTRUCTURE_API bool appendText(const QString&) noexcept;
private:
    bool appendLocked(const QString&);
    bool trim(const QString&,qint64);
    bool archive();
    QString directory_, action_, path_;
    std::function<LoggingSettings()> settings_;
    std::function<QDateTime()> clock_;
    std::mutex gate_;
};
}
