#pragma once

#include "ControllerFeedback.h"

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace RetroMediaLab::Emulation::HostProtocol {
// Use complete messages from the transport with these synchronous codecs.
inline constexpr qint32 MaximumBlobLength = 512 * 1024 * 1024;
inline constexpr qint32 VideoSlotCapacity = 128 * 1024 * 1024;
inline constexpr qint32 VideoSlotCount = 2;
inline constexpr qint32 VideoMapCapacity = VideoSlotCapacity * VideoSlotCount;
inline constexpr qint32 MinimumResizableVideoSlotCapacity = 64 * 1024;

// The host owns the reader and gives the writer the same name. Serialize access
// via the frame request and response before either side starts the next exchange.
class RETROMEDIALAB_EMULATION_API SharedVideoMemory final {
public:
    static std::unique_ptr<SharedVideoMemory> createReader(const QString& name);
    static std::unique_ptr<SharedVideoMemory> openWriter(const QString& name);
    static std::unique_ptr<SharedVideoMemory> resizableReader(const QString& namePrefix,
        qint32 minimumSlotCapacity = MinimumResizableVideoSlotCapacity);
    static std::unique_ptr<SharedVideoMemory> resizableWriter(const QString& namePrefix,
        qint32 minimumSlotCapacity = MinimumResizableVideoSlotCapacity);
    ~SharedVideoMemory();
    bool readFrame(QIODevice&, std::optional<VideoFrame>&);
    bool writeFrame(QIODevice&, const VideoFrame*);

private:
    struct Data;
    explicit SharedVideoMemory(std::unique_ptr<Data>);
    std::unique_ptr<Data> data_;
};

// Keys and buttons use the hosted-core protocol identifiers.
struct CorePointer {
    qint32 deltaX = 0, deltaY = 0, wheel = 0;
    bool left = false, right = false, middle = false, extendedButton1 = false, extendedButton2 = false;
    qint32 horizontalWheel = 0;
};

struct CoreController {
    quint32 buttons = 0;
    qint16 leftX = 0, leftY = 0, rightX = 0, rightY = 0, leftTrigger = 0, rightTrigger = 0;
    QString deviceId;
    QHash<QString, float> controls;
};

struct CoreInput {
    QSet<qint32> keys;
    std::optional<QHash<qint32, quint16>> hostCharacters;
    CorePointer pointer;
    QList<CoreController> controllers;
};

using AudioBufferStatus = AudioOutputBufferStatus;

struct RuntimeStatus {
    std::optional<VideoStandard> region;
    qint32 bufferedAudioFrames = 0;
    qint64 audioOverruns = 0, audioUnderruns = 0;
};

struct DiskSelection {
    qint32 diskCount = 0;
    qint32 currentDiskIndex = -1;
};

RETROMEDIALAB_EMULATION_API bool writeHeader(QIODevice&, qint32 version, quint8 code);
RETROMEDIALAB_EMULATION_API std::optional<quint8> readHeader(QIODevice&, qint32 expectedVersion);
RETROMEDIALAB_EMULATION_API bool writeBytes(QIODevice&, QByteArrayView);
RETROMEDIALAB_EMULATION_API std::optional<QByteArray> readBytes(QIODevice&);
// A null QString is absent. An engaged result can contain either a null or an empty string.
RETROMEDIALAB_EMULATION_API bool writeOptionalString(QIODevice&, const QString&);
RETROMEDIALAB_EMULATION_API std::optional<QString> readOptionalString(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeString(QIODevice&, const QString&);
RETROMEDIALAB_EMULATION_API std::optional<QString> readString(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeInput(QIODevice&, const CoreInput&);
RETROMEDIALAB_EMULATION_API std::optional<CoreInput> readInput(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeAudioBufferStatus(QIODevice&, const std::optional<AudioBufferStatus>&);
RETROMEDIALAB_EMULATION_API bool readAudioBufferStatus(QIODevice&, std::optional<AudioBufferStatus>&);
RETROMEDIALAB_EMULATION_API bool writeRuntimeStatus(QIODevice&, const RuntimeStatus&);
RETROMEDIALAB_EMULATION_API std::optional<RuntimeStatus> readRuntimeStatus(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeDiskSelection(QIODevice&, const DiskSelection&);
RETROMEDIALAB_EMULATION_API std::optional<DiskSelection> readDiskSelection(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeLedStates(QIODevice&, const QHash<qint32, bool>&);
RETROMEDIALAB_EMULATION_API std::optional<QHash<qint32, bool>> readLedStates(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeControllerFeedback(QIODevice&, const QList<ControllerFeedbackState>&);
RETROMEDIALAB_EMULATION_API std::optional<QList<ControllerFeedbackState>> readControllerFeedback(QIODevice&);
RETROMEDIALAB_EMULATION_API bool writeFrame(QIODevice&, const VideoFrame*);
// False indicates a malformed or incomplete message. A successful absent frame clears the destination.
RETROMEDIALAB_EMULATION_API bool readFrame(QIODevice&, std::optional<VideoFrame>&);
RETROMEDIALAB_EMULATION_API bool writeAudio(QIODevice&, const QList<AudioChunk>&);
RETROMEDIALAB_EMULATION_API std::optional<QList<AudioChunk>> readAudio(QIODevice&);
}
