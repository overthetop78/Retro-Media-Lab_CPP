#pragma once
#include "InfrastructureApi.h"
#include "HardwareConstants.h"
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <QJsonObject>
#include <QJsonValue>
#include <QVector>
#include <QString>
namespace RetroMediaLab {
struct HardwareCategory { const char* id; const char* key; };
RETROMEDIALAB_INFRASTRUCTURE_API const QVector<HardwareCategory>& hardwareCategories();
struct HardwareDevice {
    QString id, category, backend, name, address, controller;
    QString size = HardwareConstants::DefaultSize;
    QString density = HardwareConstants::UnknownValue;
    QString selection;
    int rpm = HardwareConstants::NoRpm;
    int sampleRate = HardwareConstants::DefaultRate;
    int channels = HardwareConstants::DefaultChannels;
    int channel = HardwareConstants::DefaultChannel;
    bool available = false; // Runtime discovery state, never restored as a persisted capability.
    bool operator==(const HardwareDevice&) const = default;
};
struct HardwareSettings {
    QString gwPath, previousGwPath, dtcPath;
    QString floppyBackend = HardwareConstants::Greaseweazle;
    QVector<HardwareDevice> devices;
    bool operator==(const HardwareSettings&) const = default;
    RETROMEDIALAB_INFRASTRUCTURE_API QJsonObject toJson() const;
    RETROMEDIALAB_INFRASTRUCTURE_API static HardwareSettings fromJson(const QJsonValue&);
    RETROMEDIALAB_INFRASTRUCTURE_API bool saveDevice(HardwareDevice);
    RETROMEDIALAB_INFRASTRUCTURE_API void forgetDevice(const QString& id);
    RETROMEDIALAB_INFRASTRUCTURE_API void applyFloppyDiscovery(const QString& backend,const QVector<Infrastructure::HostDevice>&);
    RETROMEDIALAB_INFRASTRUCTURE_API void applyMediaDiscovery(const QString& category,const QVector<Infrastructure::HostDevice>&,const QVector<Infrastructure::DiskController>& = {});
    RETROMEDIALAB_INFRASTRUCTURE_API bool saveDiskInterface(const Infrastructure::DiskController&);
    RETROMEDIALAB_INFRASTRUCTURE_API bool saveDetectedDevice(const QString& category,const Infrastructure::HostDevice&);
    RETROMEDIALAB_INFRASTRUCTURE_API void selectGwPath(const QString&);
};
}
