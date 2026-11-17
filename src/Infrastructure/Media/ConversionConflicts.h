#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/MediaEngine/ConversionPlanning.h>
#include <QMap>
#include <QString>
namespace RetroMediaLab::Infrastructure {
enum class ConversionConflict {Overwrite,Skip,Number};
using ConversionConflictDecisions=QMap<QString,ConversionConflict>;
RETROMEDIALAB_INFRASTRUCTURE_API std::vector<MediaEngine::ConversionOutput> resolveConversionConflicts(std::span<const MediaEngine::ConversionOutput>,const ConversionConflictDecisions&,const QString& source);
}
