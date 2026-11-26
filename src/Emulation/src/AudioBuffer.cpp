#include <RetroMediaLab/Emulation/AudioBuffer.h>
#include <algorithm>
#include <bit>

namespace RetroMediaLab::Emulation {
namespace {
constexpr int BufferedDurationDivisor = 5;

int maximumBufferedFrames(int sampleRate) {
    return std::max(1, sampleRate / BufferedDurationDivisor);
}

bool validSamples(int frames, int sampleRate, quint32 layout, std::size_t size) {
    return frames >= 0 && sampleRate > 0 && layout && !(layout & ~quint32(Surround71))
        && quint64(frames) * std::popcount(layout) == size;
}
}

bool AudioBuffer::enqueue(AudioChunk chunk) {
    if (!validSamples(chunk.frameCount, chunk.sampleRate, chunk.channelLayout,
                      std::size_t(chunk.interleavedSamples.size()))) return false;
    const auto maximum = maximumBufferedFrames(chunk.sampleRate);
    const bool trimmed = chunk.frameCount > maximum;
    if (trimmed) {
        chunk.interleavedSamples = chunk.interleavedSamples.sliced(
            qsizetype(chunk.frameCount - maximum) * std::popcount(chunk.channelLayout));
        chunk.frameCount = maximum;
    }
    push(std::move(chunk), maximum, trimmed);
    return true;
}

std::optional<AudioChunk> AudioBuffer::enqueueNative(std::span<const qint16> samples, int frames,
                                                    int sampleRate, quint64 sequence, quint32 layout,
                                                    std::chrono::nanoseconds timestamp) {
    if (!validSamples(frames, sampleRate, layout, samples.size())) return {};
    const auto maximum = maximumBufferedFrames(sampleRate);
    const auto retained = std::min(frames, maximum);
    const auto tail = samples.last(std::size_t(retained) * std::popcount(layout));
    AudioChunk chunk{QList<qint16>(tail.begin(), tail.end()), sampleRate, retained, sequence, timestamp, layout};
    push(chunk, maximum, retained < frames);
    return chunk;
}

void AudioBuffer::push(AudioChunk chunk, int maximumFrames, bool trimmed) {
    const std::lock_guard lock(mutex_);
    chunks_.push_back(std::move(chunk));
    bufferedFrames_ += chunks_.back().frameCount;
    overrunCount_ += trimmed;
    while (bufferedFrames_ > maximumFrames) {
        bufferedFrames_ -= chunks_.front().frameCount;
        chunks_.pop_front();
        ++overrunCount_;
    }
}

std::optional<AudioChunk> AudioBuffer::pop(bool countUnderrun) {
    const std::lock_guard lock(mutex_);
    if (chunks_.empty()) {
        underrunCount_ += countUnderrun;
        return {};
    }
    auto chunk = std::move(chunks_.front());
    chunks_.pop_front();
    bufferedFrames_ -= chunk.frameCount;
    return chunk;
}

std::optional<AudioChunk> AudioBuffer::dequeue() { return pop(true); }
std::optional<AudioChunk> AudioBuffer::dequeueNative() { return pop(false); }

void AudioBuffer::clear() {
    const std::lock_guard lock(mutex_);
    chunks_.clear();
    bufferedFrames_ = 0;
}

int AudioBuffer::bufferedFrames() const {
    const std::lock_guard lock(mutex_);
    return int(bufferedFrames_);
}

quint64 AudioBuffer::overrunCount() const {
    const std::lock_guard lock(mutex_);
    return overrunCount_;
}

quint64 AudioBuffer::underrunCount() const {
    const std::lock_guard lock(mutex_);
    return underrunCount_;
}
}
