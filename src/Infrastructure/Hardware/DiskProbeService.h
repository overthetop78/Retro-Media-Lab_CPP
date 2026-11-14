#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <QJsonObject>
namespace RetroMediaLab::Infrastructure {
RETROMEDIALAB_INFRASTRUCTURE_API DiskAccessReport inspectDiskInterface(const QString& controller,const QString& disk);
RETROMEDIALAB_INFRASTRUCTURE_API DiskAccessReport runDiskProbe(const QString& controller,const QString& disk);
RETROMEDIALAB_INFRASTRUCTURE_API QJsonObject diskProbeReportToJson(const DiskAccessReport&);
RETROMEDIALAB_INFRASTRUCTURE_API DiskAccessReport diskProbeReportFromJson(const QJsonObject&);
#ifdef _WIN32
DiskAccessReport requestElevatedDiskProbe(const QString& controller,const QString& disk,DiskAccessReport fallback);
#endif
}
