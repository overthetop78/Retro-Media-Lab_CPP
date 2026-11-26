#pragma once

#include "HostedCoreTransport.h"
#include <QJsonArray>
#include <functional>

namespace RetroMediaLab::Emulation {
struct HostedCoreStatus {
    double framesPerSecond = 0;
    qint32 sampleRate = 0;
    qint32 minimumAudioLatencyMs = 0;
    HostProtocol::RuntimeStatus runtime;
    QHash<qint32, bool> leds;
    QList<ControllerFeedbackState> controllerFeedback;
    bool supportsSaveStates = false;
    bool shutdownRequested = false;
    std::optional<HostProtocol::DiskSelection> diskSelection;

    RETROMEDIALAB_EMULATION_API QHash<MediaKind, bool> firstSlotActivity(bool includeOpticalDisc) const;
};

struct HostedCoreInfo {
    QString sha256;
    QString name;
    QString version;
    QStringList contentExtensions;
    QJsonArray options;
};

enum class HostedMediaDiskIndex { Slot, MountOrder, SlotIndexOrMountOrder, SlotIndexOrFirstDisk };

struct HostedCoreSessionOptions {
    HostedCoreStartOptions transport;
    qint32 version;
    // The initialization reply always carries state support. Some hosts repeat it per frame.
    bool frameStateSupport = false;
    bool frameDiskSelection = false;
    bool shutdownAndFeedback = true;
    bool initializationDiskSelection = false;
    HostedMediaDiskIndex insertDiskIndex = HostedMediaDiskIndex::Slot;
    bool stopClosesHost = false;
};

enum class HostedCoreResult { Success, InvalidState, InvalidRequest, CommunicationFailure, InvalidReply, RemoteError };

struct HostedCoreOperation {
    HostedCoreResult result;
    QJsonObject error;
    explicit operator bool() const { return result == HostedCoreResult::Success; }
};

struct HostedCoreFrame {
    QList<AudioChunk> audio;
    std::optional<HostProtocol::DiskSelection> diskSelection;
};

struct HostedCoreDiskImage {
    qint32 index;
    QString path;
    QString label;
};

struct HostedCoreDiskReport {
    qint32 imageCount = 0;
    qint32 currentIndex = -1;
    bool ejected = true;
    QList<HostedCoreDiskImage> images;
};

// A core client is confined to its emulation thread for its entire lifetime.
class RETROMEDIALAB_EMULATION_API HostedCoreSession final {
public:
    explicit HostedCoreSession(HostedCoreSessionOptions);
    ~HostedCoreSession();

    // The executable selects the initialization payload layout.
    HostedCoreOperation initialize(QByteArrayView payload, int timeoutMs, std::stop_token = {});
    HostedCoreOperation restart(QByteArrayView initializationPayload, int timeoutMs, std::stop_token = {});
    HostedCoreOperation hardReset(int timeoutMs, std::stop_token = {});
    HostedCoreOperation softReset(int timeoutMs, std::stop_token = {});
    HostedCoreOperation stop(int timeoutMs, std::stop_token = {});
    HostedCoreOperation runFrame(const HostProtocol::CoreInput&, const std::optional<HostProtocol::AudioBufferStatus>&,
        HostedCoreFrame&, int timeoutMs, std::stop_token = {});
    HostedCoreOperation execute(HostedCoreCommand, QByteArrayView payload, QByteArray& response,
        int timeoutMs, std::stop_token = {});
    HostedCoreOperation saveState(QByteArray& state, int timeoutMs, std::stop_token = {});
    HostedCoreOperation loadState(QByteArrayView state, int timeoutMs, std::stop_token = {});
    HostedCoreOperation insertMedia(const QJsonObject& media, int timeoutMs, std::stop_token = {});
    HostedCoreOperation ejectMedia(HostedMediaSlot, int timeoutMs, std::stop_token = {});
    HostedCoreOperation saveMediaChanges(HostedMediaSlot, int timeoutMs, std::stop_token = {});
    HostedCoreOperation hasUnsavedMediaChanges(HostedMediaSlot, bool& changed, int timeoutMs, std::stop_token = {});
    HostedCoreOperation setOption(const QString& key, const QString& value, int timeoutMs, std::stop_token = {});
    HostedCoreOperation setControllerPortDevice(qint32 port, qint32 device, int timeoutMs, std::stop_token = {});
    HostedCoreOperation selectDisk(qint32 index, int timeoutMs, std::stop_token = {});
    HostedCoreOperation getDiskStatus(HostedCoreDiskReport&, int timeoutMs, std::stop_token = {});
    void close() noexcept;

    bool initialized() const noexcept;
    HostProcessState hostProcessState() const noexcept;
    std::optional<qint64> hostProcessId() const noexcept;
    const HostedCoreInfo& info() const noexcept;
    const HostedCoreStatus& status() const noexcept;
    CoreRuntimeStatus coreRuntimeStatus() const noexcept;
    QList<ControllerFeedbackState> controllerFeedback() const;
    const QStringList& diagnostics() const noexcept;
    std::shared_ptr<const VideoFrame> latestFrame() const noexcept;

private:
    HostedCoreOperation request(HostedCoreCommand, QByteArrayView, QByteArray&, int, std::stop_token);
    HostedCoreOperation executeEmptyReply(HostedCoreCommand, const std::function<bool(QIODevice&)>&,
        int, std::stop_token);
    HostedCoreOperation invalidReply();
    void clearOutputs(bool shutdownRequested) noexcept;

    HostedCoreSessionOptions options_;
    std::unique_ptr<HostedCoreTransport> transport_;
    bool initialized_ = false;
    bool hostFaulted_ = false;
    std::optional<qint64> hostProcessId_;
    HostedCoreInfo info_;
    HostedCoreStatus status_;
    QStringList diagnostics_;
    std::shared_ptr<const VideoFrame> latestFrame_;
};

namespace HostProtocol {
// Initialization layout used by the native hosts with an emulator identifier.
RETROMEDIALAB_EMULATION_API bool writeInitialization(QIODevice&, const QString& corePath, qint32 emulator,
    const QString& sessionDirectory, const QString& saveDirectory, const QJsonObject& configuration);
RETROMEDIALAB_EMULATION_API bool writeInitialization(QIODevice&, const QString& corePath,
    const QString& sessionDirectory, const QString& saveDirectory, const QJsonObject& configuration);
}
}
