#include <RetroMediaLab/Emulation/NativeCoreFrames.h>
#include <RetroMediaLab/Emulation/HostProtocol.h>
#include <RetroMediaLab/Emulation/NativeCoreLibrary.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace {
constexpr std::size_t MaximumFramesPerBatch = 64 * 1024;
constexpr int StereoSampleCount = 2;
constexpr auto VideoErrorKey = "Emulation.Error.EmulatorRejected";
constexpr int VideoFrequencyPrecision = 6;
constexpr auto NtscName = "NTSC";
constexpr auto PalName = "PAL";
constexpr auto SecamName = "SECAM";
constexpr auto VideoFrequencyFormat = "%1 Hz";
constexpr auto VideoTimingFormat = "%1 (%2)";
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

int nativeFrameLength(unsigned height, std::size_t pitch) {
    if (height && pitch > std::size_t(std::numeric_limits<int>::max()) / height)
        throw std::overflow_error(VideoErrorKey);
    return int(pitch * height);
}

void copyNativeFrameRows(const void* pixels, std::span<char> destination, unsigned height, std::size_t pitch) {
    const auto length = std::size_t(nativeFrameLength(height, pitch));
    if (!pixels || !length || destination.size() < length)
        throw std::invalid_argument(VideoErrorKey);
    std::memcpy(destination.data(), pixels, length);
}

QByteArray copyNativeFramePixels(const void* pixels, unsigned height, std::size_t pitch) {
    if (!pixels || !height || !pitch) throw std::invalid_argument(VideoErrorKey);
    QByteArray frame(nativeFrameLength(height, pitch), Qt::Uninitialized);
    copyNativeFrameRows(pixels, {frame.data(), std::size_t(frame.size())}, height, pitch);
    return frame;
}

QString formatVideoStandard(VideoStandard standard) {
    switch (standard) {
    case VideoStandard::Ntsc: return QString::fromLatin1(NtscName);
    case VideoStandard::Pal: return QString::fromLatin1(PalName);
    case VideoStandard::Secam: return QString::fromLatin1(SecamName);
    }
    throw std::out_of_range(VideoErrorKey);
}

QString formatVideoFrequency(double frequencyHz) {
    if (!std::isfinite(frequencyHz) || frequencyHz <= 0) throw std::out_of_range(VideoErrorKey);
    auto value = QString::number(frequencyHz, 'f', VideoFrequencyPrecision);
    while (value.endsWith(u'0')) value.chop(1);
    if (value.endsWith(u'.')) value.chop(1);
    return QString::fromLatin1(VideoFrequencyFormat).arg(value);
}

QString formatVideoTiming(VideoStandard standard, double frequencyHz) {
    return QString::fromLatin1(VideoTimingFormat).arg(formatVideoStandard(standard), formatVideoFrequency(frequencyHz));
}

NativeCoreVideoBufferSet::NativeCoreVideoBufferSet(int bufferCount) {
    if (bufferCount <= 0) throw std::invalid_argument(VideoErrorKey);
    buffers_.resize(bufferCount);
}

std::span<char> NativeCoreVideoBufferSet::rent(int byteCount) {
    if (byteCount < 0) throw std::invalid_argument(VideoErrorKey);
    auto& buffer = buffers_[nextBuffer_];
    buffer.resizeForOverwrite(byteCount);
    auto result = std::span(buffer.data(), std::size_t(byteCount));
    nextBuffer_ = (nextBuffer_ + 1) % buffers_.size();
    return result;
}

QByteArray NativeCoreVideoBufferSet::copyFramePixels(const void* pixels, unsigned height, std::size_t pitch) {
    if (!pixels || !height || !pitch) throw std::invalid_argument(VideoErrorKey);
    const auto index = nextBuffer_;
    copyNativeFrameRows(pixels, rent(nativeFrameLength(height, pitch)), height, pitch);
    return buffers_.at(index);
}

void NativeCoreVideoBufferSet::clear() {
    for (auto& buffer : buffers_) buffer.clear();
    nextBuffer_ = 0;
}

void NativeCoreVideoFrames::setPixelFormat(PixelFormat format) {
    const std::lock_guard lock(mutex_);
    format_ = format;
}

void NativeCoreVideoFrames::setAspectRatio(float aspectRatio) {
    const std::lock_guard lock(mutex_);
    aspectRatio_ = aspectRatio;
}

void NativeCoreVideoFrames::applyGeometry(const NativeCoreGeometry& geometry) {
    if (std::isfinite(geometry.aspectRatio) && geometry.aspectRatio > 0) {
        setAspectRatio(geometry.aspectRatio);
    } else if (geometry.baseHeight > 0) {
        setAspectRatio(static_cast<float>(geometry.baseWidth) / static_cast<float>(geometry.baseHeight));
    }
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
                frame->pixels = buffers_.copyFramePixels(pixels, height, pitch);
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
    buffers_.clear();
}

std::exception_ptr NativeCoreVideoFrames::failure() const {
    const std::lock_guard lock(mutex_);
    return failure_;
}
}
