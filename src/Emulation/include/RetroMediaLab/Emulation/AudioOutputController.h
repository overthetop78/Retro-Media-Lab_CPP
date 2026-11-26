#pragma once

#include "EmulationExport.h"
#include "Sdk.h"
#include <mutex>
#include <span>
#include <vector>

namespace RetroMediaLab::Emulation {

class IAudioOutput {
public:
    virtual ~IAudioOutput() = default;
    virtual std::optional<AudioOutputBufferStatus> bufferStatus() const = 0;
    virtual bool trySetMinimumLatency(int milliseconds) = 0;
    virtual void start(int sampleRate, quint32 channelLayout) = 0;
    virtual void write(std::span<const qint16> samples) = 0;
    virtual void flush() = 0;
    virtual void stop() = 0;
};

class RETROMEDIALAB_EMULATION_API AudioOutputController final {
public:
    using Factory = std::function<std::unique_ptr<IAudioOutput>()>;

    // The caller keeps a borrowed output alive. Factory outputs belong to the controller.
    explicit AudioOutputController(IAudioOutput* output = nullptr, Factory factory = {});
    ~AudioOutputController();

    bool isMuted() const;
    float volume() const;
    std::optional<AudioOutputBufferStatus> bufferStatus();
    bool trySetMinimumLatency(int milliseconds);
    void start(int sampleRate, quint32 channelLayout = Stereo);
    void write(const AudioChunk& chunk);
    void setMuted(bool muted);
    void setVolume(float volume);
    void pause();
    void resume();
    void reset();
    void replaceFactory(Factory factory);
    void stop();

private:
    bool ensureStarted();
    void flush() noexcept;
    void stopOutput() noexcept;
    void dropOutput() noexcept;

    mutable std::mutex mutex_;
    Factory factory_;
    std::unique_ptr<IAudioOutput> ownedOutput_;
    IAudioOutput* output_;
    std::vector<qint16> volumeBuffer_;
    int sampleRate_ = 0;
    quint32 channelLayout_ = Stereo;
    int minimumLatencyMs_ = 0;
    static constexpr float FullVolume = 1.0f;
    float volume_ = FullVolume;
    bool muted_ = false;
    bool paused_ = false;
    bool stopped_ = false;
    bool started_ = false;
    bool unavailable_ = false;
};

}
