#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <HardwareDiscoveryConstants.h>
#define NOMINMAX
#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>

namespace RetroMediaLab::Infrastructure {
namespace {
using namespace HardwareDiscoveryConstants;
struct CloseHandleOnExit {
    void operator()(void* handle) const noexcept {
        if(handle && handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
    }
};
struct Packet {
    SCSI_PASS_THROUGH request{};
    std::array<UCHAR, StorageSenseBytes> sense{};
    std::array<UCHAR, StorageDataBytes> data{};
};
using Cdb = std::array<UCHAR, AtaCdbBytes>;
bool identify(HANDLE device, const Cdb& cdb, UCHAR cdbLength, ULONG bytes,
              Packet& packet, StorageCommandReport& report) {
    packet = {};
    report.scsiStatus = 0; report.nativeError = 0;
    packet.request.Length = sizeof(SCSI_PASS_THROUGH);
    packet.request.CdbLength = cdbLength;
    packet.request.SenseInfoLength = StorageSenseBytes;
    packet.request.DataIn = SCSI_IOCTL_DATA_IN;
    packet.request.DataTransferLength = bytes;
    packet.request.TimeOutValue = StorageTimeoutSeconds;
    packet.request.DataBufferOffset = offsetof(Packet, data);
    packet.request.SenseInfoOffset = offsetof(Packet, sense);
    std::copy(cdb.begin(), cdb.end(), packet.request.Cdb);
    DWORD returned = 0;
    // Windows requires synchronous pass-through and read/write handle rights.
    // Every CDB built here requests data IN; none changes media or configuration.
    if(!DeviceIoControl(device, IOCTL_SCSI_PASS_THROUGH, &packet, sizeof(packet),
                        &packet, sizeof(packet), &returned, nullptr)) {
        report.nativeError = GetLastError();
        return false;
    }
    report.nativeError = 0;
    report.scsiStatus = packet.request.ScsiStatus;
    return !report.scsiStatus && packet.request.DataTransferLength >= bytes
        && returned >= offsetof(Packet, data) + bytes;
}
StorageCommandState commandState(bool verified, const StorageCommandReport& report) {
    if(verified) return StorageCommandState::Verified;
    if(report.nativeError == ERROR_ACCESS_DENIED || report.nativeError == ERROR_PRIVILEGE_NOT_HELD) return StorageCommandState::AccessDenied;
    if(report.scsiStatus) return StorageCommandState::Rejected;
    return StorageCommandState::Failed;
}
bool validIdentify(const Packet& packet, QString& model) {
    // Reject empty/erased replies and require a printable ATA model string.
    QByteArray text;
    for(int offset = AtaModelOffset; offset < AtaModelOffset + AtaModelBytes; offset += SatFlags) {
        text.append(static_cast<char>(packet.data[offset + 1]));
        text.append(static_cast<char>(packet.data[offset]));
    }
    for(const auto byte : text) if(byte < MinimumAscii || static_cast<unsigned char>(byte) > MaximumAscii) return false;
    model = QString::fromLatin1(text).trimmed();
    return !model.isEmpty() && packet.data[0] != MaximumByte;
}
}
StorageCommandReport queryStorageCommands(const HostDevice& selected) {
    StorageCommandReport report;
    std::unique_ptr<void, CloseHandleOnExit> device(CreateFileW(
        reinterpret_cast<LPCWSTR>(selected.address.utf16()), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr));
    if(device.get() == INVALID_HANDLE_VALUE) { report.nativeError = GetLastError(); report.inquiryError=report.ataError=report.nativeError;report.inquiryState = report.ataIdentifyState = commandState(false,report); return report; }
    Packet packet;
    Cdb cdb{};
    cdb[0] = InquiryOpcode;
    cdb[CdbLengthLow] = InquiryBytes;
    report.inquiryVerified = identify(device.get(), cdb, InquiryCdbBytes, InquiryBytes, packet, report);
    report.inquiryState = commandState(report.inquiryVerified, report);
    report.inquiryError=report.nativeError;report.inquiryStatus=report.scsiStatus;
    if(report.inquiryVerified) report.identifiedModel = QString::fromLatin1(
        reinterpret_cast<const char*>(packet.data.data() + InquiryVendorOffset), InquiryModelBytes).trimmed();

    if(selected.parentId.startsWith(JMicronBridgeId, Qt::CaseInsensitive)) {
        report.transport = QString::fromLatin1(JMicronTransport);
        cdb = {};
        cdb[0] = JMicronOpcode; cdb[1] = JMicronDataIn;
        cdb[CdbLengthLow] = 1;
        cdb[CdbSectorCount] = JMicronPortHigh; cdb[CdbRegisterLow] = JMicronPortLow;
        cdb[CdbCommand] = JMicronReadRegister;
        if(!identify(device.get(), cdb, AtaCdbBytes, 1, packet, report)) { report.ataIdentifyState = commandState(false,report);report.ataError=report.nativeError;report.ataStatus=report.scsiStatus; return report; }
        const auto ports = packet.data[0] & JMicronPortMask;
        // An ambiguous dual-port bridge requires explicit routing, never guess.
        if(ports != JMicronPortZero && ports != JMicronPortOne) return report;
        cdb = {};
        cdb[0] = JMicronOpcode; cdb[1] = JMicronDataIn;
        cdb[CdbLengthHigh] = StorageDataBytes >> BigEndianShift;
        cdb[CdbSectorCount] = 1;
        cdb[CdbDevice] = ports == JMicronPortZero ? AtaMaster : AtaSlave;
        cdb[CdbCommand] = AtaIdentifyOpcode;
    } else {
        // Standard SAT identification. A rejected command means unverified,
        // not that the drive cannot read foreign filesystems.
        report.transport = QString::fromLatin1(SatTransport);
        cdb = {}; cdb[0] = SatOpcode; cdb[1] = AtaPioIn;
        cdb[SatFlags] = SatDataInBlocks; cdb[SatCount] = 1; cdb[SatCommand] = AtaIdentifyOpcode;
    }
    QString model;
    report.ataIdentifyVerified = identify(device.get(), cdb, AtaCdbBytes, StorageDataBytes, packet, report)
        && validIdentify(packet, model);
    report.ataIdentifyState = commandState(report.ataIdentifyVerified, report);
    report.ataError=report.nativeError;report.ataStatus=report.scsiStatus;
    if(report.ataIdentifyVerified) report.identifiedModel = model;
    return report;
}
}
