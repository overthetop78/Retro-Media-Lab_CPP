#include <RetroMediaLab/Emulation/AudioProcessing.h>
#include <RetroMediaLab/Emulation/AudioOutputController.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace RetroMediaLab::Emulation::AudioProcessing {
namespace {
constexpr int BufferedDurationDivisor = 5;
constexpr float SilentVolume = 0.0f;
constexpr float RoundingMidpoint = 0.5f;
constexpr auto AudioErrorKey = "Emulation.Error.EmulatorRejected";

qint16 roundedSample(qint16 sample, float volume) {
    const auto scaled = sample * volume;
    const auto magnitude = std::abs(scaled);
    auto rounded = static_cast<int>(magnitude);
    const auto fraction = magnitude - static_cast<float>(rounded);
    if (fraction > RoundingMidpoint || (fraction == RoundingMidpoint && rounded % 2 != 0))
        ++rounded;
    return static_cast<qint16>(scaled < 0 ? -rounded : rounded);
}
}

bool hasInvalidConfiguredLatency(QStringView value) {
    bool valid = false;
    const auto latency = value.toInt(&valid);
    return !valid || std::ranges::find(LatencyChoicesMs, latency) == LatencyChoicesMs.end();
}

bool hasInvalidConfiguredVolume(QStringView value, int minimum, int maximum) {
    bool valid = false;
    const auto volume = value.toInt(&valid);
    return !valid || volume < minimum || volume > maximum;
}

QString resolveRuntimeOutput(const QMap<QString, QString>& options, const QString& key,
                             const QString& defaultOutput) {
    const auto output = options.value(key);
    return output == defaultOutput ? QString{} : output;
}

int resolveRuntimeLatency(const QMap<QString, QString>& options, const QString& key) {
    bool valid = false;
    const auto latency = options.value(key).toInt(&valid);
    return valid ? latency : DefaultLatencyMs;
}

int maximumBufferedFrames(int sampleRate) noexcept {
    return std::max(1, sampleRate / BufferedDurationDivisor);
}

AudioChunk retainNewestFrames(AudioChunk chunk, int maximumFrames) {
    if (maximumFrames < 0) throw std::invalid_argument(AudioErrorKey);
    if (chunk.frameCount <= maximumFrames) return chunk;
    const auto channels = std::popcount(chunk.channelLayout);
    if (!channels || quint64(chunk.frameCount) * channels != quint64(chunk.interleavedSamples.size()))
        throw std::invalid_argument(AudioErrorKey);
    chunk.interleavedSamples = std::move(chunk.interleavedSamples).sliced(
        qsizetype(chunk.frameCount - maximumFrames) * channels);
    chunk.frameCount = maximumFrames;
    return chunk;
}

QList<qint16> copyNativeBatch(const qint16* samples, std::size_t frameCount, unsigned channelCount) {
    if (!channelCount) throw std::invalid_argument(AudioErrorKey);
    if (frameCount > std::size_t(std::numeric_limits<int>::max()) / channelCount)
        throw std::overflow_error(AudioErrorKey);
    if (!frameCount) return {};
    if (!samples) throw std::invalid_argument(AudioErrorKey);
    return {samples, samples + frameCount * channelCount};
}

float normalizeVolume(float volume) noexcept {
    return std::isnan(volume) ? SilentVolume : std::clamp(volume, SilentVolume, FullVolume);
}

std::span<const qint16> applyVolume(std::span<const qint16> samples, float volume, std::vector<qint16>& buffer) {
    volume = normalizeVolume(volume);
    if (volume >= FullVolume) return samples;
    buffer.resize(samples.size());
    std::transform(samples.begin(), samples.end(), buffer.begin(),
                   [volume](qint16 sample) { return roundedSample(sample, volume); });
    return buffer;
}

void writeVolume(IAudioOutput* output, float volume, std::span<const qint16> samples) {
    if (!output) return;
    if (volume >= FullVolume) {
        output->write(samples);
        return;
    }
    if (std::isnan(volume)) volume = SilentVolume;
    std::vector<qint16> scaled(samples.size());
    std::transform(samples.begin(), samples.end(), scaled.begin(), [volume](qint16 sample) {
        if (sample == 0) return qint16{};
        return static_cast<qint16>(std::clamp(sample * volume, float(std::numeric_limits<qint16>::min()),
                                                                    float(std::numeric_limits<qint16>::max())));
    });
    output->write(scaled);
}
}
