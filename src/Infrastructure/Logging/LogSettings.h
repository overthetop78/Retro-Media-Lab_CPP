#pragma once
#include "InfrastructureApi.h"
#include "LogConstants.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QString>
#include <array>
namespace RetroMediaLab {
struct LogAction { const char* id; const char* label; };
RETROMEDIALAB_INFRASTRUCTURE_API const std::array<LogAction, LogConstants::ActionCount>& logActions();
struct ActionLogSettings {
    bool enabled = true;
    int maximumKilobytes = LogConstants::DefaultMaximumKiB;
    bool keepArchives = false;
    bool operator==(const ActionLogSettings&) const = default;
};
struct LoggingSettings {
    ActionLogSettings defaults;
    QMap<QString, ActionLogSettings> actions;
    RETROMEDIALAB_INFRASTRUCTURE_API ActionLogSettings forAction(const QString&) const;
    RETROMEDIALAB_INFRASTRUCTURE_API void validate() const;
    RETROMEDIALAB_INFRASTRUCTURE_API QJsonObject toJson() const;
    RETROMEDIALAB_INFRASTRUCTURE_API static LoggingSettings fromJson(const QJsonValue&);
    bool operator==(const LoggingSettings&) const = default;
};
}
