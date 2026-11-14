#include <HardwareDiscoveryConstants.h>
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
StorageCommandReport queryStorageCommands(const HostDevice&) {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
QVector<HostDevice> discoverHardware(const QString&) {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
OpticalCapabilities queryOpticalCapabilities(const QString&) {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
OpticalCapabilities parseOpticalConfiguration(const QByteArray&) {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
QVector<DiskController> diskControllers() {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
DiskAccessReport queryDiskAccess(const QString&,bool) {
    throw std::runtime_error(HardwareDiscoveryConstants::Unsupported);
}
}
