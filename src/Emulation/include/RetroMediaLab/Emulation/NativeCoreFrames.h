#pragma once

#include "AudioBuffer.h"
#include <cstddef>
#include <exception>
#include <mutex>

namespace RetroMediaLab::Emulation {
// An adapter forwards its native callbacks to the owner for its session.
class RETROMEDIALAB_EMULATION_API NativeCoreAudioFrames final {
public:
    void setSampleRate(int sampleRate);
    void sample(qint16 left, qint16 right) noexcept;
    std::size_t batch(const qint16* interleaved, std::size_t frames) noexcept;

    std::optional<AudioChunk> latestChunk() const;
    std::optional<AudioChunk> takeAudio();
    int bufferedFrames() const;
    quint64 overrunCount() const;
    quint64 underrunCount() const;
    void clear();
    std::exception_ptr failure() const;

private:
    mutable std::mutex mutex_;
    AudioBuffer buffer_;
    std::optional<AudioChunk> latest_;
    std::exception_ptr failure_;
    int sampleRate_ = 0;
    quint64 sequence_ = 0;
};

class RETROMEDIALAB_EMULATION_API NativeCoreVideoFrames final {
public:
    void setPixelFormat(PixelFormat format);
    void setAspectRatio(float aspectRatio);
    void submit(const void* pixels, unsigned width, unsigned height, std::size_t pitch) noexcept;

    std::shared_ptr<const VideoFrame> latestFrame() const;
    void clear();
    std::exception_ptr failure() const;

private:
    mutable std::mutex mutex_;
    const std::chrono::steady_clock::time_point started_ = std::chrono::steady_clock::now();
    std::shared_ptr<const VideoFrame> latest_;
    std::exception_ptr failure_;
    PixelFormat format_ = PixelFormat::Xrgb8888;
    float aspectRatio_ = 0;
    quint64 sequence_ = 0;
};
}
