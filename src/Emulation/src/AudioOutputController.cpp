#include <RetroMediaLab/Emulation/AudioOutputController.h>
#include <RetroMediaLab/Emulation/AudioProcessing.h>

namespace RetroMediaLab::Emulation {

AudioOutputController::AudioOutputController(IAudioOutput* output, Factory factory)
    : factory_(std::move(factory)), output_(output) {}

AudioOutputController::~AudioOutputController() { dropOutput(); }

bool AudioOutputController::isMuted() const {
    const std::lock_guard lock(mutex_);
    return muted_;
}

float AudioOutputController::volume() const {
    const std::lock_guard lock(mutex_);
    return volume_;
}

std::optional<AudioOutputBufferStatus> AudioOutputController::bufferStatus() {
    const std::lock_guard lock(mutex_);
    try {
        auto status = output_ ? output_->bufferStatus() : std::nullopt;
        if (status) status->active &= started_ && !stopped_ && !paused_ && !muted_ && !unavailable_;
        return status;
    } catch (...) {
        dropOutput();
        unavailable_ = true;
        return std::nullopt;
    }
}

bool AudioOutputController::trySetMinimumLatency(int milliseconds) {
    const std::lock_guard lock(mutex_);
    if (milliseconds < 0) return false;
    if (milliseconds == minimumLatencyMs_) return true;
    try {
        if (output_ && !output_->trySetMinimumLatency(milliseconds)) return false;
        minimumLatencyMs_ = milliseconds;
        return true;
    } catch (...) {
        dropOutput();
        unavailable_ = true;
        return false;
    }
}

void AudioOutputController::start(int sampleRate, quint32 channelLayout) {
    const std::lock_guard lock(mutex_);
    if (started_ && (sampleRate != sampleRate_ || channelLayout != channelLayout_)) stopOutput();
    sampleRate_ = sampleRate;
    channelLayout_ = channelLayout;
    stopped_ = false;
    unavailable_ = false;
    ensureStarted();
}

void AudioOutputController::write(const AudioChunk& chunk) {
    const std::lock_guard lock(mutex_);
    if (stopped_ || paused_ || muted_) return;
    if (chunk.sampleRate != sampleRate_ || chunk.channelLayout != channelLayout_) {
        sampleRate_ = chunk.sampleRate;
        channelLayout_ = chunk.channelLayout;
        stopOutput();
    }
    if (!ensureStarted()) return;
    const auto samples = AudioProcessing::applyVolume({chunk.interleavedSamples.constData(),
        static_cast<std::size_t>(chunk.interleavedSamples.size())}, volume_, volumeBuffer_);
    try {
        output_->write(samples);
    } catch (...) {
        dropOutput();
        if (!ensureStarted()) return;
        try {
            output_->write(samples);
        } catch (...) {
            dropOutput();
            unavailable_ = true;
        }
    }
}

void AudioOutputController::setMuted(bool muted) {
    const std::lock_guard lock(mutex_);
    muted_ = muted;
    if (muted) flush();
    else {
        unavailable_ = false;
        ensureStarted();
    }
}

void AudioOutputController::setVolume(float volume) {
    const std::lock_guard lock(mutex_);
    volume_ = AudioProcessing::normalizeVolume(volume);
}

void AudioOutputController::pause() {
    const std::lock_guard lock(mutex_);
    paused_ = true;
    flush();
    stopOutput();
}

void AudioOutputController::resume() {
    const std::lock_guard lock(mutex_);
    paused_ = false;
    unavailable_ = false;
    ensureStarted();
}

void AudioOutputController::reset() {
    const std::lock_guard lock(mutex_);
    flush();
}

void AudioOutputController::replaceFactory(Factory factory) {
    const std::lock_guard lock(mutex_);
    factory_ = std::move(factory);
    unavailable_ = false;
    dropOutput();
    ensureStarted();
}

void AudioOutputController::stop() {
    const std::lock_guard lock(mutex_);
    stopped_ = true;
    paused_ = false;
    stopOutput();
}

bool AudioOutputController::ensureStarted() {
    if (stopped_ || paused_ || muted_ || unavailable_ || sampleRate_ <= 0) return false;
    if (started_) return true;
    try {
        if (!output_ && factory_) {
            ownedOutput_ = factory_();
            output_ = ownedOutput_.get();
        }
        if (output_) {
            if (minimumLatencyMs_ > 0) output_->trySetMinimumLatency(minimumLatencyMs_);
            output_->start(sampleRate_, channelLayout_);
            started_ = true;
            return true;
        }
    } catch (...) {
        dropOutput();
    }
    unavailable_ = true;
    return false;
}

void AudioOutputController::flush() noexcept {
    try {
        if (output_) output_->flush();
    } catch (...) {
        dropOutput();
    }
}

void AudioOutputController::stopOutput() noexcept {
    if (!std::exchange(started_, false)) return;
    try { output_->stop(); } catch (...) {}
}

void AudioOutputController::dropOutput() noexcept {
    stopOutput();
    output_ = nullptr;
    ownedOutput_.reset();
}

}
