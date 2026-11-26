#pragma once

#include "HostProtocol.h"
#include <QJsonObject>
#include <exception>
#include <functional>

namespace RetroMediaLab::Emulation {
enum class HostedVideoMapping { Fixed, Resizable };

struct HostedCoreStartOptions {
    QString executable;
    // An empty directory runs the host beside its executable.
    QString workingDirectory;
    QString command;
    int connectionTimeoutMs = 10'000;
    int gracefulShutdownTimeoutMs = 2'000;
    HostedVideoMapping videoMapping = HostedVideoMapping::Fixed;
    qint32 minimumVideoSlotCapacity = HostProtocol::MinimumResizableVideoSlotCapacity;
};

enum class HostedCoreCommand : quint8 {
    Initialize = 1, RunFrame = 2, HardReset = 3, Stop = 4, InsertMedia = 5,
    EjectMedia = 6, SaveState = 7, LoadState = 8, SetOption = 9, SelectDisk = 10,
    Dispose = 11, SaveMediaChanges = 12, GetDiskStatus = 13, HasUnsavedMediaChanges = 14,
    SetControllerPortDevice = 15, SoftReset = 16
};

enum class HostedMediaCategory : qint32 { FloppyDrive, HardDisk, CompactDiscDrive, CartridgeSlot, TapeDrive, MemoryCard };

struct HostedMediaSlot {
    HostedMediaCategory category;
    qint32 index;
};

struct HostedCoreReply {
    QByteArray payload;
    std::optional<QJsonObject> error;
};

// A session owns one transport and performs its exchanges on the owning thread.
class RETROMEDIALAB_EMULATION_API HostedCoreTransport final {
public:
    static std::unique_ptr<HostedCoreTransport> start(const HostedCoreStartOptions&, std::stop_token = {});
    static std::unique_ptr<HostedCoreTransport> start(const HostedCoreStartOptions&, qint64& startedProcessId, std::stop_token);
    ~HostedCoreTransport();
    HostedCoreTransport(const HostedCoreTransport&) = delete;
    HostedCoreTransport& operator=(const HostedCoreTransport&) = delete;

    // The request includes its protocol header. The returned response excludes
    // the four-byte length prefix. Any interrupted exchange closes the pipe.
    std::optional<QByteArray> exchange(QByteArrayView request, int timeoutMs, std::stop_token = {});
    // The version and module-specific payload layouts belong to the selected core.
    // A core's error response is distinct from a failed exchange.
    std::optional<HostedCoreReply> command(qint32 version, HostedCoreCommand, QByteArrayView payload,
        int timeoutMs, std::stop_token = {});
    bool connected() const noexcept;
    bool isProcessRunning() const noexcept;
    qint64 processId() const noexcept;
    HostProtocol::SharedVideoMemory& videoMemory() noexcept;

private:
    struct Data;
    explicit HostedCoreTransport(std::unique_ptr<Data>);
    std::unique_ptr<Data> data_;
};

namespace HostProtocol {
using HostedCommandHandler = std::function<bool(HostedCoreCommand, QIODevice& request, QIODevice& response)>;
using HostedErrorDescription = std::function<QJsonObject(std::exception_ptr)>;

// Run on the dedicated host process thread. Consume each command's arguments before replying.
// A false handler result ends the connection on an invalid or incomplete request.
// An exception becomes a structured failure response, replacing any partial success.
RETROMEDIALAB_EMULATION_API bool runHostedCoreConnection(const QString& pipeName, qint32 version,
    int connectionTimeoutMs, const HostedCommandHandler&, const HostedErrorDescription&, std::stop_token = {});

RETROMEDIALAB_EMULATION_API bool writeMediaSlot(QIODevice&, HostedMediaSlot);
RETROMEDIALAB_EMULATION_API std::optional<HostedMediaSlot> readMediaSlot(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeOption(QIODevice&, const QString& id, const QString& value);
RETROMEDIALAB_EMULATION_API bool writeIndex(QIODevice&, qint32 index);
RETROMEDIALAB_EMULATION_API bool writeControllerPortDevice(QIODevice&, qint32 port, qint32 device);
RETROMEDIALAB_EMULATION_API std::optional<bool> readMediaChanges(QIODevice&);
}
}
