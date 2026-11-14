#pragma once
#include "InfrastructureApi.h"
#include <QString>
#include <QVector>
#include <QByteArray>
namespace RetroMediaLab::Infrastructure {
enum class AudioInputKind { Other, Line, Microphone, Digital };
enum class HostDeviceState { Active, Disabled, Unplugged };
struct HostDevice {
    QString id, name, address, backend;
    bool available = true;
    AudioInputKind inputKind = AudioInputKind::Other;
    HostDeviceState state = HostDeviceState::Active;
    QString parentId, parentName, connection, parentLocation, parentDriver;
};
struct DiskController {
    QString id, name, location, driver, connection;
};
enum class DiskReadState { NotTested, Passed, MediaInformationUnavailable, AccessDenied, Failed };
enum class StorageCommandState { NotTested, Verified, Rejected, AccessDenied, Failed };
enum class DiskElevationState { NotRequired, Completed, Cancelled, Failed };
struct StorageCommandReport {
    bool inquiryVerified = false, ataIdentifyVerified = false;
    StorageCommandState inquiryState = StorageCommandState::NotTested;
    StorageCommandState ataIdentifyState = StorageCommandState::NotTested;
    QString transport, identifiedModel;
    quint32 nativeError = 0;
    quint8 scsiStatus = 0;
    quint32 inquiryError = 0, ataError = 0;
    quint8 inquiryStatus = 0, ataStatus = 0;
};
struct DiskAccessReport {
    QString connection, diskName;
    QString controllerName,controllerLocation,controllerDriver;
    quint64 capacityBytes = 0;
    quint32 logicalSectorBytes = 0, physicalSectorBytes = 0;
    quint32 maximumTransferBytes = 0, alignmentMask = 0;
    bool adapterPropertiesKnown = false, commandQueueing = false;
    quint32 maximumPhysicalPages = 0;
    quint16 busMajorVersion = 0, busMinorVersion = 0;
    bool adapterUsesPio = false, acceleratedTransfer = false;
    bool addressKnown = false;
    quint8 portNumber = 0, pathId = 0, targetId = 0, lun = 0;
    DiskReadState readState = DiskReadState::NotTested;
    int sampledSectors = 0;
    bool rawAccessVerified = false;
    StorageCommandReport commands;
    DiskElevationState elevation = DiskElevationState::NotRequired;
    quint32 nativeError = 0;
    quint64 failedSector = 0;
};
struct OpticalCapabilities {
    bool cdRead = false, dvdRead = false, bdRead = false;
    bool cdDetailsKnown = false, c2 = false, cdText = false;
    QVector<int> profiles;
    QVector<int> features;
};
// Enumerates device metadata only. Does not read media, mount volumes or start recording.
RETROMEDIALAB_INFRASTRUCTURE_API QVector<HostDevice> discoverHardware(const QString& category);
// Only queries advertised MMC features; does not write, eject or extract media.
RETROMEDIALAB_INFRASTRUCTURE_API OpticalCapabilities queryOpticalCapabilities(const QString& address);
RETROMEDIALAB_INFRASTRUCTURE_API OpticalCapabilities parseOpticalConfiguration(const QByteArray&);
RETROMEDIALAB_INFRASTRUCTURE_API QVector<DiskController> diskControllers();
RETROMEDIALAB_INFRASTRUCTURE_API DiskAccessReport queryDiskAccess(const QString& address,bool readSample);
// Identification commands only, routed through the device class driver. No data-out CDB.
RETROMEDIALAB_INFRASTRUCTURE_API StorageCommandReport queryStorageCommands(const HostDevice& device);
}
