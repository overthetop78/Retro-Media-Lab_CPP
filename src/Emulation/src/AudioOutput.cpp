#include "AudioOutput.h"

#include <QIODevice>
#include <QAudioDevice>
#include <QMediaDevices>
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

namespace RetroMediaLab::Emulation {
namespace {
constexpr int MaxChunksPerPoll = 8;
constexpr int MillisecondsPerSecond = 1000;
constexpr int MinimumConfiguredLatencyMs = 10;
constexpr int MaximumLatencyMs = 500;

void publish(IEmulatedMachine& machine, std::optional<AudioOutputBufferStatus> status) noexcept {
    if (auto* feedback = dynamic_cast<IMachineAudioBufferFeedback*>(&machine))
        feedback->setAudioBufferStatus(status);
}

QString outputDeviceId(const QAudioDevice& device) {
    return QString::fromLatin1(device.id().toHex());
}

QAudioDevice findOutputDevice(const QString& id) {
    if (id.isEmpty()) return QMediaDevices::defaultAudioOutput();
    const auto devices = QMediaDevices::audioOutputs();
    const auto found = std::ranges::find(devices, id, outputDeviceId);
    return found == devices.end() ? QAudioDevice{} : *found;
}
}

QList<SettingsChoice> AudioOutput::deviceChoices() {
    QList<SettingsChoice> choices;
    for (const auto& device : QMediaDevices::audioOutputs())
        choices.append({outputDeviceId(device), {}, device.description()});
    return choices;
}

void AudioOutput::clear() noexcept {
    if (sink_) sink_->stop();
    sink_.reset();
    device_ = nullptr;
    pending_.clear();
    requestedFormat_.reset();
}

void AudioOutput::stop(IEmulatedMachine& machine) noexcept {
    clear();
    publish(machine, std::nullopt);
}

void AudioOutput::append(const AudioChunk& chunk, int latencyMs, const QString& deviceId) {
    const auto channels = std::popcount(chunk.channelLayout);
    if (chunk.sampleRate <= 0 || chunk.frameCount <= 0 || channels == 0 ||
        (chunk.channelLayout & ~quint32(Surround71)) != 0 ||
        chunk.interleavedSamples.size() != qint64(channels) * chunk.frameCount) return;
    QAudioFormat format;
    format.setSampleRate(chunk.sampleRate);
    format.setChannelConfig(static_cast<QAudioFormat::ChannelConfig>(chunk.channelLayout << 1));
    format.setSampleFormat(QAudioFormat::Int16);
    if (requestedFormat_ != format || latencyMs_ != latencyMs || deviceId_ != deviceId) {
        clear();
        requestedFormat_ = format;
        latencyMs_ = latencyMs;
        deviceId_ = deviceId;
        const auto device = findOutputDevice(deviceId);
        if (device.isNull()) return;
        sink_ = std::make_unique<QAudioSink>(device, format);
        if (latencyMs_ > 0) {
            const auto frames = (qint64(chunk.sampleRate) * latencyMs_ + MillisecondsPerSecond - 1) /
                MillisecondsPerSecond;
            const auto bytes = std::min<qint64>(frames * format.bytesPerFrame(), std::numeric_limits<qsizetype>::max());
            sink_->setBufferSize(qsizetype(bytes));
        }
        device_ = sink_->start();
    }
    if (!device_) return;
    const auto length = qsizetype(sizeof(qint16)) * chunk.interleavedSamples.size();
    if (length > sink_->bufferSize() || pending_.size() > sink_->bufferSize() - length) return;
    pending_.append(reinterpret_cast<const char*>(chunk.interleavedSamples.constData()), length);
}

std::optional<AudioOutputBufferStatus> AudioOutput::status() const {
    if (!device_) return {};
    const auto format = sink_->format();
    const auto bytesPerFrame = format.bytesPerFrame();
    if (bytesPerFrame <= 0 || sink_->bufferSize() <= 0) return {};
    const auto capacity = std::min<qint64>(sink_->bufferSize() / bytesPerFrame, std::numeric_limits<qint32>::max());
    if (capacity == 0) return {};
    const auto free = std::clamp<qint64>(sink_->bytesFree(), 0, sink_->bufferSize());
    const auto buffered = std::min<qint64>((sink_->bufferSize() - free) / bytesPerFrame, capacity);
    const auto state = sink_->state();
    return AudioOutputBufferStatus{format.sampleRate(), qint32(buffered), qint32(capacity),
        state == QtAudio::ActiveState || state == QtAudio::IdleState};
}

void AudioOutput::pump(IEmulatedMachine& machine, bool audible, const ModuleAudioOutputSelection& settings) {
    if ((!audible || deviceId_ != settings.deviceId) && requestedFormat_) clear();
    const auto* feedback = dynamic_cast<const IMachineAudioBufferFeedback*>(&machine);
    const auto latency = feedback ? feedback->minimumAudioLatencyMs() : 0;
    if (latency >= 0 && latency <= MaximumLatencyMs) coreMinimumLatencyMs_ = latency;
    const auto configuredLatency = settings.latencyMs == 0 ? 0 :
        std::clamp(settings.latencyMs, MinimumConfiguredLatencyMs, MaximumLatencyMs);
    const auto outputLatency = std::max(coreMinimumLatencyMs_, configuredLatency);
    const auto volume = std::isfinite(settings.volume) ? std::clamp<qreal>(settings.volume, 0, 1) : qreal{1};
    for (int i = 0; i < MaxChunksPerPoll; ++i) {
        auto chunk = machine.takeAudio();
        if (!chunk) break;
        if (audible) append(*chunk, outputLatency, settings.deviceId);
    }
    if (sink_ && sink_->volume() != volume) sink_->setVolume(volume);
    if (device_ && !pending_.isEmpty() && sink_->bytesFree() > 0) {
        const auto sent = device_->write(pending_.constData(), std::min<qint64>(sink_->bytesFree(), pending_.size()));
        if (sent > 0) pending_.remove(0, sent);
        if (sent < 0) clear();
    }
    publish(machine, status());
}
}
