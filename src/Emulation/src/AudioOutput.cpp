#include "AudioOutput.h"

#include <RetroMediaLab/MediaEngine/DriveService.h>
#include <RetroMediaLab/MediaEngine/MechanicalSound.h>
#include <QDebug>
#include <QMetaObject>
#include <cstring>
#include <mutex>
#include <set>

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

namespace {
constexpr int MechanicalSampleRate = 44100;
constexpr int MechanicalChunkMilliseconds = 20;
constexpr int MechanicalRefillMilliseconds = MechanicalChunkMilliseconds / 2;
constexpr int MechanicalBufferedChunks = 4;
constexpr int MechanicalMaximumDrainMilliseconds = 1000;
constexpr auto MechanicalChunkMicroseconds =
    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(MechanicalChunkMilliseconds)).count();
constexpr int MonoChannels = 1;
constexpr auto MechanicalAudioFailure = "Emulation.MechanicalAudio";
}

struct MechanicalAudioOutput::Sources {
    std::mutex gate;
    std::set<std::weak_ptr<Drive>, std::owner_less<>> active;
    QTimer* timer = nullptr;
    bool scheduled = false;
};

MechanicalAudioOutput::MechanicalAudioOutput(const MediaEngine::MediaDriveService& drives)
    : sources_(std::make_shared<Sources>()) {
    sources_->timer = &timer_;
    timer_.setInterval(MechanicalRefillMilliseconds);
    QObject::connect(&timer_, &QTimer::timeout, &timer_, [this] { pump(); });
    drives.setAudioAvailable([weak = std::weak_ptr<Sources>(sources_)](auto drive) noexcept {
        try {
            const auto sources = weak.lock();
            if (!sources) return;
            std::lock_guard lock(sources->gate);
            if (!sources->timer) return;
            sources->active.insert(drive);
            if (!sources->scheduled) {
                auto* timer = sources->timer;
                sources->scheduled = QMetaObject::invokeMethod(timer, [timer] { timer->start(); }, Qt::QueuedConnection);
            }
        } catch (...) {
            qWarning("%s", MechanicalAudioFailure);
        }
    });
}

MechanicalAudioOutput::~MechanicalAudioOutput() {
    {
        std::lock_guard lock(sources_->gate);
        sources_->timer = nullptr;
    }
    timer_.stop();
    if (sink_) sink_->reset();
}

MechanicalAudioOutput::Rendered MechanicalAudioOutput::mix(
    const std::vector<std::shared_ptr<Drive>>& drives, int rate, int frames, int channels) {
    Rendered result;
    std::vector<double> mixed(static_cast<std::size_t>(frames));
    for (const auto& drive : drives) {
        try {
            const auto samples = drive->renderAudio(rate, frames);
            const auto count = std::min(samples.size(), mixed.size());
            for (std::size_t i = 0; i < count; ++i) mixed[i] += samples[i];
        } catch (...) {
            result.failed.push_back(drive);
            qWarning("%s", MechanicalAudioFailure);
        }
    }
    const auto samples = MediaEngine::protectMechanicalAudioMix(mixed);
    result.pcm.resizeForOverwrite(qsizetype(samples.size()) * channels * qsizetype(sizeof(std::int16_t)));
    auto* dest = result.pcm.data();
    for (const auto sample : samples) {
        for (int channel = 0; channel < channels; ++channel) {
            std::memcpy(dest, &sample, sizeof(sample));
            dest += sizeof(sample);
        }
    }
    return result;
}

bool MechanicalAudioOutput::openOutput() {
    const auto selected = QMediaDevices::defaultAudioOutput();
    if (selected.isNull()) return false;
    QAudioFormat format;
    format.setSampleRate(MechanicalSampleRate);
    format.setChannelCount(MonoChannels);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!selected.isFormatSupported(format)) {
        format = selected.preferredFormat();
        format.setSampleFormat(QAudioFormat::Int16);
        if (!selected.isFormatSupported(format)) return false;
    }
    sink_ = std::make_unique<QAudioSink>(selected, format);
    sink_->setBufferSize(format.bytesForDuration(MechanicalBufferedChunks * MechanicalChunkMicroseconds));
    device_ = sink_->start();
    return device_ != nullptr;
}

void MechanicalAudioOutput::finish() {
    timer_.stop();
    device_ = nullptr;
    if (sink_) sink_->reset();
    sink_.reset();
    pending_.clear();
    drainTime_.invalidate();
    sources_->active.clear();
    sources_->scheduled = false;
}

void MechanicalAudioOutput::pump() noexcept {
    try {
        refill();
    } catch (...) {
        qWarning("%s", MechanicalAudioFailure);
        std::lock_guard lock(sources_->gate);
        finish();
    }
}

void MechanicalAudioOutput::refill() {
    if (rendering_.valid()) {
        if (rendering_.wait_for(std::chrono::seconds::zero()) != std::future_status::ready) return;
        auto rendered = rendering_.get();
        pending_ = std::move(rendered.pcm);
        if (!rendered.failed.empty()) {
            std::lock_guard lock(sources_->gate);
            for (const auto& failed : rendered.failed) sources_->active.erase(failed);
        }
    }
    if (sink_ && sink_->error() != QtAudio::NoError) {
        std::lock_guard lock(sources_->gate);
        finish();
        return;
    }
    if (device_ && !pending_.isEmpty()) {
        const auto count = std::min<qint64>(sink_->bytesFree(), pending_.size());
        if (count <= 0) return;
        const auto written = device_->write(pending_.constData(), count);
        if (written < 0) {
            std::lock_guard lock(sources_->gate);
            finish();
            return;
        }
        pending_.remove(0, written);
        if (!pending_.isEmpty()) return;
    }
    std::vector<std::shared_ptr<Drive>> batch;
    {
        std::lock_guard lock(sources_->gate);
        std::erase_if(sources_->active, [&batch](const auto& weak) {
            auto source = weak.lock();
            if (!source || !source->hasPendingAudio()) return true;
            batch.push_back(std::move(source));
            return false;
        });
        if (batch.empty()) {
            if (!drainTime_.isValid()) drainTime_.start();
            if (!sink_ || sink_->state() == QtAudio::IdleState ||
                drainTime_.hasExpired(MechanicalMaximumDrainMilliseconds)) finish();
            return;
        }
    }
    drainTime_.invalidate();
    if (!device_ && !openOutput()) {
        std::lock_guard lock(sources_->gate);
        finish();
        return;
    }
    const auto format = sink_->format();
    if (sink_->bytesFree() < format.bytesForDuration(MechanicalChunkMicroseconds)) return;
    rendering_ = std::async(std::launch::async,
        [batch = std::move(batch), rate = format.sampleRate(), frames = format.framesForDuration(MechanicalChunkMicroseconds),
            channels = format.channelCount()] {
            return mix(batch, rate, frames, channels);
        });
}
}
