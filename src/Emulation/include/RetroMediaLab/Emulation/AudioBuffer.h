#pragma once

#include "EmulationExport.h"
#include "Sdk.h"
#include <deque>
#include <mutex>
#include <span>

namespace RetroMediaLab::Emulation {
class RETROMEDIALAB_EMULATION_API AudioBuffer final {
public:
    bool enqueue(AudioChunk chunk);
    std::optional<AudioChunk> enqueueNative(std::span<const qint16> samples, int frames, int sampleRate,
                                            quint64 sequence, quint32 layout = Stereo,
                                            std::chrono::nanoseconds timestamp = {});
    std::optional<AudioChunk> dequeue();
    std::optional<AudioChunk> dequeueNative();
    void clear();

    int bufferedFrames() const;
    quint64 overrunCount() const;
    quint64 underrunCount() const;

private:
    void push(AudioChunk chunk, int maximumFrames, bool trimmed);
    std::optional<AudioChunk> pop(bool countUnderrun);

    mutable std::mutex mutex_;
    std::deque<AudioChunk> chunks_;
    qint64 bufferedFrames_ = 0;
    quint64 overrunCount_ = 0;
    quint64 underrunCount_ = 0;
};
}
