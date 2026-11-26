#include <RetroMediaLab/Emulation/NativeCoreFrames.h>
#include <RetroMediaLab/Emulation/HostProtocol.h>
#include <algorithm>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace {
constexpr std::size_t MaximumFramesPerBatch = 64 * 1024;
constexpr int StereoSampleCount = 2;
constexpr auto VideoErrorKey = "Emulation.Error.EmulatorRejected";
}

void NativeCoreAudioFrames::setSampleRate(int sampleRate) {
    const std::lock_guard lock(mutex_);
    sampleRate_ = sampleRate;
}

void NativeCoreAudioFrames::sample(qint16 left, qint16 right) noexcept {
    const qint16 frame[]{left, right};
    batch(frame, 1);
}

std::size_t NativeCoreAudioFrames::batch(const qint16* samples, std::size_t frames) noexcept {
    if (!samples || frames == 0 || frames > MaximumFramesPerBatch) return 0;
    try {
        const std::lock_guard lock(mutex_);
        try {
            auto chunk = buffer_.enqueueNative({samples, frames * StereoSampleCount}, int(frames),
                                                sampleRate_, sequence_ + 1);
            if (!chunk) return 0;
            latest_ = std::move(chunk);
            ++sequence_;
        }
        catch (...) {
            if (!failure_) failure_ = std::current_exception();
            return 0;
        }
        return frames;
    } catch (...) {
        return 0;
    }
}

std::optional<AudioChunk> NativeCoreAudioFrames::latestChunk() const {
    const std::lock_guard lock(mutex_);
    return latest_;
}

std::optional<AudioChunk> NativeCoreAudioFrames::takeAudio() {
    return buffer_.dequeueNative();
}

int NativeCoreAudioFrames::bufferedFrames() const {
    return buffer_.bufferedFrames();
}

quint64 NativeCoreAudioFrames::overrunCount() const {
    return buffer_.overrunCount();
}

quint64 NativeCoreAudioFrames::underrunCount() const {
    return buffer_.underrunCount();
}

void NativeCoreAudioFrames::clear() {
    const std::lock_guard lock(mutex_);
    buffer_.clear();
    latest_.reset();
}

std::exception_ptr NativeCoreAudioFrames::failure() const {
    const std::lock_guard lock(mutex_);
    return failure_;
}

void NativeCoreVideoFrames::setPixelFormat(PixelFormat format) {
    const std::lock_guard lock(mutex_);
    format_ = format;
}

void NativeCoreVideoFrames::setAspectRatio(float aspectRatio) {
    const std::lock_guard lock(mutex_);
    aspectRatio_ = aspectRatio;
}

void NativeCoreVideoFrames::submit(const void* pixels, unsigned width, unsigned height, std::size_t pitch) noexcept {
    try {
        const std::lock_guard lock(mutex_);
        try {
            if (!pixels && !latest_) return;
            if (pixels && (!width || !height || !pitch)) return;

            std::shared_ptr<VideoFrame> frame;
            if (pixels) {
                const auto bytesPerPixel = format_ == PixelFormat::Xrgb8888 ? sizeof(quint32) : sizeof(quint16);
                if (width > pitch / bytesPerPixel || pitch > std::size_t(HostProtocol::VideoSlotCapacity) / height)
                    throw std::runtime_error(VideoErrorKey);
                frame = std::make_shared<VideoFrame>();
                frame->pixels = QByteArray(static_cast<const char*>(pixels), qsizetype(pitch * height));
                frame->width = int(width);
                frame->height = int(height);
                frame->pitch = int(pitch);
                frame->pixelFormat = format_;
                frame->aspectRatio = aspectRatio_;
            } else {
                frame = std::make_shared<VideoFrame>(*latest_);
            }
            frame->sequence = ++sequence_;
            frame->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - started_);
            latest_ = std::move(frame);
        } catch (...) {
            if (!failure_) failure_ = std::current_exception();
        }
    } catch (...) {
        // Never propagate an exception through the core callback.
    }
}

std::shared_ptr<const VideoFrame> NativeCoreVideoFrames::latestFrame() const {
    const std::lock_guard lock(mutex_);
    return latest_;
}

void NativeCoreVideoFrames::clear() {
    const std::lock_guard lock(mutex_);
    latest_.reset();
}

std::exception_ptr NativeCoreVideoFrames::failure() const {
    const std::lock_guard lock(mutex_);
    return failure_;
}
}
