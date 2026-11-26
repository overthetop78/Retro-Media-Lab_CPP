#include <RetroMediaLab/Emulation/HostProtocol.h>
#include <QDataStream>
#include <QIODevice>
#include <QSharedMemory>
#include <QtEndian>
#include <QUuid>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>

namespace RetroMediaLab::Emulation::HostProtocol {
namespace {
constexpr int NanosecondsPerTick = 100;
constexpr qint32 MaximumAudioChunkCount = 1024;
constexpr auto MaximumFeedbackCount = ControllerFeedbackCommands::MaximumPortCount * 2;
constexpr qint32 MaximumKeys = 512;
constexpr qint32 MaximumControllers = ControllerFeedbackCommands::MaximumPortCount;
constexpr qint32 MaximumControllerControls = 4096;
constexpr qint32 MaximumLeds = 256;
constexpr qint32 AbsentCharacterMap = -1;
constexpr qint32 UnknownRegion = -1;
constexpr qint32 NtscRegion = 0, PalRegion = 1, SecamRegion = 2;
constexpr unsigned VarintBits = 7;
constexpr unsigned VarintPayload = 0x7f;
constexpr unsigned VarintMore = 0x80;
constexpr unsigned VarintLastShift = 28;
constexpr unsigned VarintLastMaximum = 7;

void configure(QDataStream& io) {
    io.setByteOrder(QDataStream::LittleEndian);
    io.setFloatingPointPrecision(QDataStream::SinglePrecision);
}

bool okay(const QDataStream& io) {
    return io.status() == QDataStream::Ok;
}

bool writePayload(QDataStream& io, QByteArrayView data) {
    return data.isEmpty() || io.writeRawData(data.data(), data.size()) == data.size();
}

std::optional<QByteArray> readPayload(QDataStream& io, qint32 length) {
    if (length < 0 || length > MaximumBlobLength) return {};
    if (!io.device()->isSequential() && length > io.device()->bytesAvailable()) return {};
    QByteArray data("");
    constexpr qint32 BlockSize = 64 * 1024;
    for (qint32 offset = 0; offset < length;) {
        const auto size = std::min(BlockSize, length - offset);
        data.resize(offset + size);
        if (io.readRawData(data.data() + offset, size) != size) return {};
        offset += size;
    }
    return data;
}

bool writeSizedPayload(QDataStream& io, QByteArrayView payload) {
    if (payload.size() > MaximumBlobLength) return false;
    io << static_cast<qint32>(payload.size());
    return okay(io) && writePayload(io, payload);
}

std::optional<QByteArray> readSizedPayload(QDataStream& io) {
    qint32 length{};
    io >> length;
    return okay(io) ? readPayload(io, length) : std::nullopt;
}

bool writeUtf8(QDataStream& io, QByteArrayView bytes) {
    auto length = static_cast<quint32>(bytes.size());
    while (length >= VarintMore) {
        io << static_cast<quint8>((length & VarintPayload) | VarintMore);
        length >>= VarintBits;
    }
    io << static_cast<quint8>(length);
    return okay(io) && writePayload(io, bytes);
}

std::optional<QString> readUtf8(QDataStream& io) {
    quint32 length{};
    for (unsigned shift = 0; shift <= VarintLastShift; shift += VarintBits) {
        quint8 byte{};
        io >> byte;
        if (!okay(io) || (shift == VarintLastShift && byte > VarintLastMaximum)) return {};
        length |= static_cast<quint32>(byte & VarintPayload) << shift;
        if ((byte & VarintMore) == 0) {
            const auto bytes = readPayload(io, static_cast<qint32>(length));
            if (!bytes) return {};
            return bytes->isEmpty() ? QStringLiteral("") : QString::fromUtf8(*bytes);
        }
    }
    return {};
}

bool validController(const CoreController& controller) {
    if (controller.controls.size() > MaximumControllerControls || controller.deviceId.toUtf8().size() > MaximumBlobLength)
        return false;
    QSet<QString> seen;
    for (auto control = controller.controls.cbegin(); control != controller.controls.cend(); ++control) {
        const auto folded = control.key().toCaseFolded();
        if (seen.contains(folded) || control.key().toUtf8().size() > MaximumBlobLength) return false;
        seen.insert(folded);
    }
    return true;
}

bool writeCoreController(QDataStream& io, const CoreController& controller) {
    io << controller.buttons << controller.leftX << controller.leftY << controller.rightX << controller.rightY
        << controller.leftTrigger << controller.rightTrigger;
    if (!okay(io) || !writeUtf8(io, controller.deviceId.toUtf8())) return false;
    auto names = controller.controls.keys();
    std::ranges::sort(names);
    io << qint32(names.size());
    for (const auto& name : names) {
        if (!writeUtf8(io, name.toUtf8())) return false;
        io << controller.controls.value(name);
    }
    return okay(io);
}

std::optional<CoreController> readCoreController(QDataStream& io) {
    CoreController controller;
    io >> controller.buttons >> controller.leftX >> controller.leftY >> controller.rightX >> controller.rightY
        >> controller.leftTrigger >> controller.rightTrigger;
    auto deviceId = readUtf8(io);
    if (!deviceId) return {};
    controller.deviceId = std::move(*deviceId);
    qint32 count{};
    io >> count;
    if (!okay(io) || count < 0 || count > MaximumControllerControls) return {};
    QHash<QString, QString> originalNames;
    for (qint32 index = 0; index < count; ++index) {
        const auto name = readUtf8(io);
        if (!name) return {};
        float value{};
        io >> value;
        if (!okay(io)) return {};
        const auto folded = name->toCaseFolded();
        if (!originalNames.contains(folded)) originalNames.insert(folded, *name);
        controller.controls.insert(originalNames.value(folded), value);
    }
    return controller;
}

bool validFeedback(const QList<ControllerFeedbackState>& states) {
    if (states.size() > MaximumFeedbackCount) return false;
    QSet<QPair<int, QString>> seen;
    for (const auto& state : states) {
        if (state.port < 0 || static_cast<unsigned>(state.port) >= ControllerFeedbackCommands::MaximumPortCount ||
            (state.commandId != ControllerFeedbackCommands::Strong && state.commandId != ControllerFeedbackCommands::Weak) ||
            !std::isfinite(state.strength) || state.strength < 0.0f || state.strength > 1.0f ||
            seen.contains({state.port, state.commandId})) return false;
        seen.insert({state.port, state.commandId});
    }
    return true;
}

std::optional<std::chrono::nanoseconds> timestamp(qint64 ticks) {
    using Time = std::chrono::nanoseconds;
    if (ticks < Time::min().count() / NanosecondsPerTick || ticks > Time::max().count() / NanosecondsPerTick)
        return {};
    return Time(ticks * NanosecondsPerTick);
}

bool validFrameSize(int width, int height, int pitch, qsizetype length) {
    return width > 0 && height > 0 && pitch > 0 && qint64(pitch) * height == length && length <= MaximumBlobLength;
}

bool validPixelFormat(qint32 format) {
    return format == static_cast<qint32>(PixelFormat::Rgb565) ||
        format == static_cast<qint32>(PixelFormat::Xrgb8888) || format == static_cast<qint32>(PixelFormat::Rgb1555);
}

ScanMode scanMode(qint32 mode) {
    return mode == static_cast<qint32>(ScanMode::EvenField) ? ScanMode::EvenField :
        mode == static_cast<qint32>(ScanMode::OddField) ? ScanMode::OddField : ScanMode::Progressive;
}

bool canWriteFrame(const VideoFrame& frame) {
    return validFrameSize(frame.width, frame.height, frame.pitch, frame.pixels.size()) &&
        validPixelFormat(static_cast<qint32>(frame.pixelFormat));
}

void writeFrameMetadata(QDataStream& io, const VideoFrame& frame) {
    io << qint32(frame.width) << qint32(frame.height) << qint32(frame.pitch) << qint32(frame.pixelFormat)
        << frame.aspectRatio << frame.sequence << qint64(frame.timestamp.count() / NanosecondsPerTick)
        << qint32(frame.scanMode);
}

bool readFrameMetadata(QDataStream& io, VideoFrame& frame) {
    qint32 pixelFormat{}, scan{};
    qint64 ticks{};
    io >> frame.width >> frame.height >> frame.pitch >> pixelFormat >> frame.aspectRatio
        >> frame.sequence >> ticks >> scan;
    const auto time = timestamp(ticks);
    if (!okay(io) || !time || !validPixelFormat(pixelFormat)) return false;
    frame.timestamp = *time;
    frame.pixelFormat = static_cast<PixelFormat>(pixelFormat);
    frame.scanMode = scanMode(scan);
    return true;
}

QNativeIpcKey videoMemoryKey(const QString& name) {
#ifdef Q_OS_WIN
    return QNativeIpcKey(name, QNativeIpcKey::Type::Windows);
#else
    return QSharedMemory::platformSafeKey(name);
#endif
}

bool validPcm(int sampleRate, int frames, quint32 layout, qsizetype samples) {
    return sampleRate > 0 && frames >= 0 && layout != 0 && (layout & ~quint32(Surround71)) == 0 &&
        qint64(frames) * std::popcount(layout) == samples && samples <= MaximumBlobLength / sizeof(qint16);
}
}

bool writeHeader(QIODevice& device, qint32 version, quint8 code) {
    QDataStream io(&device);
    configure(io);
    io << version << code;
    return okay(io);
}

std::optional<quint8> readHeader(QIODevice& device, qint32 expectedVersion) {
    QDataStream io(&device);
    configure(io);
    qint32 version{};
    io >> version;
    if (!okay(io) || version != expectedVersion) return {};
    quint8 code{};
    io >> code;
    return okay(io) ? std::optional{code} : std::nullopt;
}

bool writeBytes(QIODevice& device, QByteArrayView bytes) {
    QDataStream io(&device);
    configure(io);
    return writeSizedPayload(io, bytes);
}

std::optional<QByteArray> readBytes(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    return readSizedPayload(io);
}

bool writeOptionalString(QIODevice& device, const QString& value) {
    const auto bytes = value.toUtf8();
    if (bytes.size() > MaximumBlobLength) return false;
    QDataStream io(&device);
    configure(io);
    io << !value.isNull();
    return okay(io) && (value.isNull() || writeUtf8(io, bytes));
}

std::optional<QString> readOptionalString(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    bool present{};
    io >> present;
    if (!okay(io)) return {};
    return present ? readUtf8(io) : std::optional{QString{}};
}

bool writeString(QIODevice& device, const QString& value) {
    const auto bytes = value.toUtf8();
    if (bytes.size() > MaximumBlobLength) return false;
    QDataStream io(&device);
    configure(io);
    return writeUtf8(io, bytes);
}

std::optional<QString> readString(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    return readUtf8(io);
}

bool writeInput(QIODevice& device, const CoreInput& input) {
    if (input.keys.size() > MaximumKeys || (input.hostCharacters && input.hostCharacters->size() > MaximumKeys) ||
        input.controllers.size() > MaximumControllers || !std::ranges::all_of(input.controllers, validController))
        return false;
    QDataStream io(&device);
    configure(io);
    auto keys = input.keys.values();
    std::ranges::sort(keys);
    io << qint32(keys.size());
    for (const auto key : keys) io << key;
    io << (input.hostCharacters ? qint32(input.hostCharacters->size()) : AbsentCharacterMap);
    if (input.hostCharacters) {
        auto characterKeys = input.hostCharacters->keys();
        std::ranges::sort(characterKeys);
        for (const auto key : characterKeys) io << key << input.hostCharacters->value(key);
    }
    const auto& pointer = input.pointer;
    io << pointer.deltaX << pointer.deltaY << pointer.wheel << pointer.left << pointer.right << pointer.middle
        << pointer.extendedButton1 << pointer.extendedButton2 << pointer.horizontalWheel;
    io << qint32(input.controllers.size());
    for (const auto& controller : input.controllers)
        if (!writeCoreController(io, controller)) return false;
    return okay(io);
}

std::optional<CoreInput> readInput(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    qint32 keyCount{};
    io >> keyCount;
    if (!okay(io) || keyCount < 0 || keyCount > MaximumKeys) return {};
    CoreInput input;
    for (qint32 index = 0; index < keyCount; ++index) {
        qint32 key{};
        io >> key;
        input.keys.insert(key);
    }
    qint32 characterCount{};
    io >> characterCount;
    if (!okay(io) || characterCount < AbsentCharacterMap || characterCount > MaximumKeys) return {};
    if (characterCount != AbsentCharacterMap) input.hostCharacters.emplace();
    for (qint32 index = 0; index < characterCount; ++index) {
        qint32 key{};
        quint16 character{};
        io >> key >> character;
        if (!okay(io) || input.hostCharacters->contains(key)) return {};
        input.hostCharacters->insert(key, character);
    }
    auto& pointer = input.pointer;
    io >> pointer.deltaX >> pointer.deltaY >> pointer.wheel >> pointer.left >> pointer.right >> pointer.middle
        >> pointer.extendedButton1 >> pointer.extendedButton2 >> pointer.horizontalWheel;
    qint32 controllerCount{};
    io >> controllerCount;
    if (!okay(io) || controllerCount < 0 || controllerCount > MaximumControllers) return {};
    input.controllers.reserve(controllerCount);
    for (qint32 index = 0; index < controllerCount; ++index) {
        auto controller = readCoreController(io);
        if (!controller) return {};
        input.controllers.append(std::move(*controller));
    }
    return input;
}

bool writeAudioBufferStatus(QIODevice& device, const std::optional<AudioBufferStatus>& status) {
    QDataStream io(&device);
    configure(io);
    io << status.has_value();
    if (status) io << status->sampleRate << status->bufferedFrames << status->capacityFrames << status->active;
    return okay(io);
}

bool readAudioBufferStatus(QIODevice& device, std::optional<AudioBufferStatus>& destination) {
    QDataStream io(&device);
    configure(io);
    bool present{};
    io >> present;
    if (!okay(io)) return false;
    if (!present) {
        destination.reset();
        return true;
    }
    AudioBufferStatus status;
    io >> status.sampleRate >> status.bufferedFrames >> status.capacityFrames >> status.active;
    if (!okay(io)) return false;
    destination = status;
    return true;
}

bool writeRuntimeStatus(QIODevice& device, const RuntimeStatus& status) {
    QDataStream io(&device);
    configure(io);
    qint32 region = UnknownRegion;
    if (status.region) {
        switch (*status.region) {
        case VideoStandard::Ntsc: region = NtscRegion; break;
        case VideoStandard::Pal: region = PalRegion; break;
        case VideoStandard::Secam: region = SecamRegion; break;
        }
    }
    io << region << status.bufferedAudioFrames << status.audioOverruns << status.audioUnderruns;
    return okay(io);
}

std::optional<RuntimeStatus> readRuntimeStatus(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    qint32 region{};
    RuntimeStatus status;
    io >> region >> status.bufferedAudioFrames >> status.audioOverruns >> status.audioUnderruns;
    if (!okay(io)) return {};
    switch (region) {
    case NtscRegion: status.region = VideoStandard::Ntsc; break;
    case PalRegion: status.region = VideoStandard::Pal; break;
    case SecamRegion: status.region = VideoStandard::Secam; break;
    }
    return status;
}

bool writeDiskSelection(QIODevice& device, const DiskSelection& selection) {
    QDataStream io(&device);
    configure(io);
    io << selection.diskCount << selection.currentDiskIndex;
    return okay(io);
}

std::optional<DiskSelection> readDiskSelection(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    DiskSelection selection;
    io >> selection.diskCount >> selection.currentDiskIndex;
    return okay(io) ? std::optional{selection} : std::nullopt;
}

bool writeLedStates(QIODevice& device, const QHash<qint32, bool>& states) {
    if (states.size() > MaximumLeds) return false;
    QDataStream io(&device);
    configure(io);
    auto ids = states.keys();
    std::ranges::sort(ids);
    io << qint32(ids.size());
    for (const auto id : ids) io << id << states.value(id);
    return okay(io);
}

std::optional<QHash<qint32, bool>> readLedStates(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    qint32 count{};
    io >> count;
    if (!okay(io) || count < 0 || count > MaximumLeds) return {};
    QHash<qint32, bool> states;
    for (qint32 index = 0; index < count; ++index) {
        qint32 id{};
        bool enabled{};
        io >> id >> enabled;
        if (!okay(io)) return {};
        states.insert(id, enabled);
    }
    return states;
}

bool writeControllerFeedback(QIODevice& device, const QList<ControllerFeedbackState>& states) {
    if (!validFeedback(states)) return false;
    QDataStream io(&device);
    configure(io);
    io << static_cast<qint32>(states.size());
    for (const auto& state : states)
        io << qint32(state.port) << (state.commandId == ControllerFeedbackCommands::Strong) << state.strength;
    return okay(io);
}

std::optional<QList<ControllerFeedbackState>> readControllerFeedback(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    qint32 count{};
    io >> count;
    if (!okay(io) || count < 0 || static_cast<unsigned>(count) > MaximumFeedbackCount) return {};
    QList<ControllerFeedbackState> states;
    states.reserve(count);
    for (int index = 0; index < count; ++index) {
        qint32 port{};
        bool strong{};
        float strength{};
        io >> port >> strong >> strength;
        states.append({port, QString(strong ? ControllerFeedbackCommands::Strong : ControllerFeedbackCommands::Weak), strength});
    }
    if (!okay(io) || !validFeedback(states)) return {};
    return states;
}

bool writeFrame(QIODevice& device, const VideoFrame* frame) {
    if (frame && !canWriteFrame(*frame)) return false;
    QDataStream io(&device);
    configure(io);
    io << (frame != nullptr);
    if (!frame) return okay(io);
    writeFrameMetadata(io, *frame);
    return okay(io) && writeSizedPayload(io, frame->pixels);
}

bool readFrame(QIODevice& device, std::optional<VideoFrame>& destination) {
    QDataStream io(&device);
    configure(io);
    bool present{};
    io >> present;
    if (!okay(io)) return false;
    if (!present) {
        destination.reset();
        return true;
    }
    VideoFrame frame;
    if (!readFrameMetadata(io, frame)) return false;
    qint32 bytes{};
    io >> bytes;
    if (!okay(io) || !validFrameSize(frame.width, frame.height, frame.pitch, bytes)) return false;
    auto pixels = readPayload(io, bytes);
    if (!pixels) return false;
    frame.pixels = std::move(*pixels);
    destination = std::move(frame);
    return true;
}

struct SharedVideoMemory::Data {
    Data(QString name, bool writable, qint32 minimum = 0)
        : prefix(std::move(name)), minimumCapacity(minimum), writer(writable) {}

    bool map(const QString& name, qint32 capacity, bool create, QSharedMemory::AccessMode mode) {
        if (segment && name == activeName) return capacity == slotCapacity;
        auto replacement = std::make_unique<QSharedMemory>(videoMemoryKey(name));
        const auto size = qsizetype(capacity) * VideoSlotCount;
        if (!(create ? replacement->create(size, mode) : replacement->attach(mode)) ||
            replacement->size() < size) return false;
        segment = std::move(replacement);
        activeName = name;
        slotCapacity = capacity;
        return true;
    }

    const QString prefix;
    const qint32 minimumCapacity;
    const bool writer;
    QString activeName;
    qint32 slotCapacity = 0;
    std::unique_ptr<QSharedMemory> segment;
};

namespace {
constexpr qsizetype VideoGenerationIdLength = 32;
constexpr auto VideoGenerationSeparator = u'-';

bool validMappingName(const QString& name) {
    return !name.trimmed().isEmpty() && !name.contains(QChar::Null);
}

bool validResizableCapacity(qint32 capacity, qint32 minimum) {
    return capacity >= minimum && capacity <= VideoSlotCapacity &&
        std::has_single_bit(static_cast<quint32>(capacity));
}

bool validGeneration(const QString& prefix, const QString& name) {
    if (name.size() != prefix.size() + 1 + VideoGenerationIdLength ||
        !name.startsWith(prefix) || name.at(prefix.size()) != VideoGenerationSeparator) return false;
    return std::ranges::all_of(QStringView(name).last(VideoGenerationIdLength), [](QChar c) {
        return (c >= u'0' && c <= u'9') || (c >= u'a' && c <= u'f') || (c >= u'A' && c <= u'F');
    });
}
}

SharedVideoMemory::SharedVideoMemory(std::unique_ptr<Data> data) : data_(std::move(data)) {}
SharedVideoMemory::~SharedVideoMemory() = default;

std::unique_ptr<SharedVideoMemory> SharedVideoMemory::createReader(const QString& name) {
    if (!validMappingName(name)) return {};
    auto data = std::make_unique<Data>(name, false);
    if (!data->map(name, VideoSlotCapacity, true, QSharedMemory::ReadOnly)) return {};
    return std::unique_ptr<SharedVideoMemory>(new SharedVideoMemory(std::move(data)));
}

std::unique_ptr<SharedVideoMemory> SharedVideoMemory::openWriter(const QString& name) {
    if (!validMappingName(name)) return {};
    auto data = std::make_unique<Data>(name, true);
    if (!data->map(name, VideoSlotCapacity, false, QSharedMemory::ReadWrite)) return {};
    return std::unique_ptr<SharedVideoMemory>(new SharedVideoMemory(std::move(data)));
}

std::unique_ptr<SharedVideoMemory> SharedVideoMemory::resizableReader(const QString& prefix, qint32 minimum) {
    if (!validMappingName(prefix) || !validResizableCapacity(minimum, 1)) return {};
    return std::unique_ptr<SharedVideoMemory>(new SharedVideoMemory(std::make_unique<Data>(prefix, false, minimum)));
}

std::unique_ptr<SharedVideoMemory> SharedVideoMemory::resizableWriter(const QString& prefix, qint32 minimum) {
    if (!validMappingName(prefix) || !validResizableCapacity(minimum, 1)) return {};
    return std::unique_ptr<SharedVideoMemory>(new SharedVideoMemory(std::make_unique<Data>(prefix, true, minimum)));
}

bool SharedVideoMemory::writeFrame(QIODevice& device, const VideoFrame* frame) {
    if (!data_->writer || (frame && (!canWriteFrame(*frame) || frame->pixels.size() > VideoSlotCapacity)))
        return false;
    QDataStream io(&device);
    configure(io);
    if (!frame) {
        io << false;
        return okay(io);
    }
    const auto slot = qint32(frame->sequence & (VideoSlotCount - 1));
    if (data_->minimumCapacity) {
        const auto capacity = qint32(std::bit_ceil(quint32(std::max<qsizetype>(
            frame->pixels.size(), data_->minimumCapacity))));
        if (capacity != data_->slotCapacity) {
            const auto name = data_->prefix + VideoGenerationSeparator + QUuid::createUuid().toString(QUuid::Id128);
            if (!data_->map(name, capacity, true, QSharedMemory::ReadWrite)) return false;
        }
    }
    auto* pixels = static_cast<char*>(data_->segment->data()) + qsizetype(slot) * data_->slotCapacity;
    std::memcpy(pixels, frame->pixels.constData(), static_cast<size_t>(frame->pixels.size()));
    io << true;
    if (data_->minimumCapacity) {
        if (!writeUtf8(io, data_->activeName.toUtf8())) return false;
        io << data_->slotCapacity;
    }
    writeFrameMetadata(io, *frame);
    io << slot << qint32(frame->pixels.size());
    return okay(io);
}

bool SharedVideoMemory::readFrame(QIODevice& device, std::optional<VideoFrame>& destination) {
    if (data_->writer) return false;
    QDataStream io(&device);
    configure(io);
    bool present{};
    io >> present;
    if (!okay(io)) return false;
    if (!present) {
        destination.reset();
        return true;
    }
    auto capacity = data_->slotCapacity;
    QString name;
    if (data_->minimumCapacity) {
        const auto generation = readUtf8(io);
        io >> capacity;
        if (!generation || !okay(io) || !validGeneration(data_->prefix, *generation) ||
            !validResizableCapacity(capacity, data_->minimumCapacity)) return false;
        name = *generation;
    }
    VideoFrame frame;
    if (!readFrameMetadata(io, frame)) return false;
    qint32 slot{}, length{};
    io >> slot >> length;
    if (!okay(io) || slot < 0 || slot >= VideoSlotCount || length > capacity ||
        !validFrameSize(frame.width, frame.height, frame.pitch, length)) return false;
    if (data_->minimumCapacity && !data_->map(name, capacity, false, QSharedMemory::ReadOnly)) return false;
    const auto* pixels = static_cast<const char*>(data_->segment->constData()) + qsizetype(slot) * capacity;
    frame.pixels = QByteArray(pixels, length);
    destination = std::move(frame);
    return true;
}

bool writeAudio(QIODevice& device, const QList<AudioChunk>& chunks) {
    if (chunks.size() > MaximumAudioChunkCount || std::ranges::any_of(chunks, [](const auto& chunk) {
            return !validPcm(chunk.sampleRate, chunk.frameCount, chunk.channelLayout, chunk.interleavedSamples.size());
        })) return false;
    QDataStream io(&device);
    configure(io);
    io << static_cast<qint32>(chunks.size());
    for (const auto& chunk : chunks) {
        const auto& samples = chunk.interleavedSamples;
        const auto bytes = qint32(samples.size() * sizeof(qint16));
        io << qint32(chunk.sampleRate) << qint32(chunk.frameCount) << chunk.channelLayout << chunk.sequence
            << qint64(chunk.timestamp.count() / NanosecondsPerTick) << bytes;
        if constexpr (std::endian::native == std::endian::little) {
            if (!writePayload(io, QByteArrayView(reinterpret_cast<const char*>(samples.constData()), bytes))) return false;
        } else {
            for (const auto sample : samples) io << sample;
        }
        if (!okay(io)) return false;
    }
    return okay(io);
}

std::optional<QList<AudioChunk>> readAudio(QIODevice& device) {
    QDataStream io(&device);
    configure(io);
    qint32 count{};
    io >> count;
    if (!okay(io) || count < 0 || count > MaximumAudioChunkCount) return {};
    QList<AudioChunk> chunks;
    chunks.reserve(count);
    for (int index = 0; index < count; ++index) {
        AudioChunk chunk;
        qint64 ticks{};
        qint32 length{};
        io >> chunk.sampleRate >> chunk.frameCount >> chunk.channelLayout >> chunk.sequence >> ticks >> length;
        const auto time = timestamp(ticks);
        constexpr qint32 BytesPerSample = sizeof(qint16);
        if (!okay(io) || !time || length < 0 || (length % BytesPerSample) != 0 ||
            !validPcm(chunk.sampleRate, chunk.frameCount, chunk.channelLayout, length / BytesPerSample)) return {};
        const auto pcm = readPayload(io, length);
        if (!pcm) return {};
        chunk.timestamp = *time;
        chunk.interleavedSamples.resize(length / BytesPerSample);
        if (length != 0) qFromLittleEndian<qint16>(pcm->constData(), chunk.interleavedSamples.size(), chunk.interleavedSamples.data());
        chunks.append(std::move(chunk));
    }
    return chunks;
}
}
