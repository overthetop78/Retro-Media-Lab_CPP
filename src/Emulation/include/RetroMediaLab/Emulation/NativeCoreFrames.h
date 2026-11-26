#pragma once

#include "AudioBuffer.h"
#include <QList>
#include <cstddef>
#include <exception>
#include <mutex>
#include <span>

namespace RetroMediaLab::Emulation {
struct NativeCoreGeometry;

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

RETROMEDIALAB_EMULATION_API int nativeFrameLength(unsigned height, std::size_t pitch);
RETROMEDIALAB_EMULATION_API void copyNativeFrameRows(const void* pixels, std::span<char> destination,
                                                    unsigned height, std::size_t pitch);
RETROMEDIALAB_EMULATION_API QByteArray copyNativeFramePixels(const void* pixels, unsigned height, std::size_t pitch);
RETROMEDIALAB_EMULATION_API QString formatVideoStandard(VideoStandard standard);
RETROMEDIALAB_EMULATION_API QString formatVideoFrequency(double frequencyHz);
RETROMEDIALAB_EMULATION_API QString formatVideoTiming(VideoStandard standard, double frequencyHz);

class RETROMEDIALAB_EMULATION_API NativeCoreVideoBufferSet final {
public:
    explicit NativeCoreVideoBufferSet(int bufferCount);
    // The writable view lasts until this slot is reused or cleared.
    std::span<char> rent(int byteCount);
    // The returned Qt buffer retains its pixels when the slot is reused.
    QByteArray copyFramePixels(const void* pixels, unsigned height, std::size_t pitch);
    void clear();

private:
    QList<QByteArray> buffers_;
    qsizetype nextBuffer_ = 0;
};

class RETROMEDIALAB_EMULATION_API NativeCoreVideoFrames final {
public:
    void setPixelFormat(PixelFormat format);
    void setAspectRatio(float aspectRatio);
    void applyGeometry(const NativeCoreGeometry& geometry);
    void submit(const void* pixels, unsigned width, unsigned height, std::size_t pitch) noexcept;

    std::shared_ptr<const VideoFrame> latestFrame() const;
    void clear();
    std::exception_ptr failure() const;

private:
    static constexpr int BufferCount = 2;
    mutable std::mutex mutex_;
    NativeCoreVideoBufferSet buffers_{BufferCount};
    const std::chrono::steady_clock::time_point started_ = std::chrono::steady_clock::now();
    std::shared_ptr<const VideoFrame> latest_;
    std::exception_ptr failure_;
    PixelFormat format_ = PixelFormat::Xrgb8888;
    float aspectRatio_ = 0;
    quint64 sequence_ = 0;
};
}
