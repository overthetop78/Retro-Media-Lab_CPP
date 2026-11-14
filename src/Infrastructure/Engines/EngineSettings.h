#pragma once
#include "InfrastructureApi.h"
#include "EngineConstants.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVector>
namespace RetroMediaLab {
struct EngineOperation {const char* id;const char* label;const char* defaultEngine;};
RETROMEDIALAB_INFRASTRUCTURE_API const QVector<EngineOperation>& engineOperations();
struct EngineSettings {
    QMap<QString,QMap<QString,QString>> selections;
    bool operator==(const EngineSettings&) const = default;
    RETROMEDIALAB_INFRASTRUCTURE_API QString engineFor(const QString& backend,const QString& operation) const;
    RETROMEDIALAB_INFRASTRUCTURE_API void select(const QString& backend,const QString& operation,const QString& engine);
    RETROMEDIALAB_INFRASTRUCTURE_API void validate() const;
    RETROMEDIALAB_INFRASTRUCTURE_API QJsonObject toJson() const;
    RETROMEDIALAB_INFRASTRUCTURE_API static EngineSettings fromJson(const QJsonValue&);
};
}
