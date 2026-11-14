#pragma once
#include "InfrastructureApi.h"
#include "ConversionConstants.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QStringList>
#include <QVector>
namespace RetroMediaLab {
struct TagDefinition { const char* value; const char* label; };
RETROMEDIALAB_INFRASTRUCTURE_API const QVector<TagDefinition>& conversionPresets();
RETROMEDIALAB_INFRASTRUCTURE_API const QVector<TagDefinition>& conversionVariables();
struct ConversionSettings {
    bool addTags = false;
    QString tagPattern = ConversionConstants::DefaultPattern;
    QStringList recentCustomTagPatterns;
    bool operator==(const ConversionSettings&) const = default;
    RETROMEDIALAB_INFRASTRUCTURE_API void rememberCustomPattern();
    RETROMEDIALAB_INFRASTRUCTURE_API QJsonObject toJson() const;
    RETROMEDIALAB_INFRASTRUCTURE_API static ConversionSettings fromJson(const QJsonValue&);
};
RETROMEDIALAB_INFRASTRUCTURE_API QString conversionExample(const QString& pattern, int index);
}
