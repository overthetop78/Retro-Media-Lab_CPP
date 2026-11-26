#pragma once
#include "InfrastructureApi.h"
#include <QString>
namespace RetroMediaLab {
RETROMEDIALAB_INFRASTRUCTURE_API QString applicationDataDirectory(const QString& applicationDirectory);
RETROMEDIALAB_INFRASTRUCTURE_API QString logsDirectory(const QString& applicationDirectory);
RETROMEDIALAB_INFRASTRUCTURE_API bool ensureLogDirectory(const QString&);
}
