#pragma once

#include "Host.h"
#include <QDateTime>
#include <functional>

namespace RetroMediaLab::Emulation {
enum class FirmwareCompatibility { Official, Compatible, PartiallyCompatible, Incompatible, Unknown };
enum class FirmwareDetectionStatus { Known, Unknown, Unreadable };

struct FirmwareCandidate {
    QString id, path, displayName, version;
    FirmwareCompatibility compatibility = FirmwareCompatibility::Unknown;
    QString destinationFieldId, explanationResourceKey;
};

template<class Definition>
struct FirmwareObservation {
    QString path;
    std::optional<qint64> sizeBytes;
    QString md5;
    FirmwareDetectionStatus detection = FirmwareDetectionStatus::Unknown;
    std::optional<Definition> definition;
    FirmwareCompatibility compatibility = FirmwareCompatibility::Unknown;
    bool isDuplicate = false;
    QString readError, sha256;
    QDateTime lastWriteTimeUtc;
    QString type;
    bool isOfficial = false;
    QString name, version;
    QStringList compatibleModels;
    bool isVerified = true;
    bool isKnown() const noexcept { return detection == FirmwareDetectionStatus::Known; }
};

class IModuleFirmwareManager {
public:
    virtual ~IModuleFirmwareManager() = default;
    virtual QString firmwareDirectory(const QString& machineId) const = 0;
    virtual QList<FirmwareCandidate> scanFirmware(const QString& machineId, const Configuration&, std::stop_token) = 0;
    virtual Configuration useFirmware(Configuration, const FirmwareCandidate&) = 0;
};

enum class FirmwareScanMode { Directory, Recursive };
enum class FirmwareScanResult { Complete, Cancelled, DirectoryError };
using FirmwareInspector = std::function<void(const QString&, std::stop_token)>;

RETROMEDIALAB_EMULATION_API quint32 firmwareCrc32(QByteArrayView) noexcept;
// A null identity or destination is unspecified. An ambiguous match has no result.
RETROMEDIALAB_EMULATION_API std::optional<FirmwareCandidate> selectFirmwareCandidate(
    const QList<FirmwareCandidate>&, const QString& candidateId = {}, const QString& destinationFieldId = {});
RETROMEDIALAB_EMULATION_API FirmwareScanResult scanFirmwareFiles(const QString& directory, FirmwareScanMode,
    const FirmwareInspector&, std::stop_token = {});
}
