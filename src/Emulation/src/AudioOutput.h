#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>
#include <QAudioSink>
#include <QElapsedTimer>
#include <QTimer>
#include <future>
#include <vector>

namespace RetroMediaLab::MediaEngine {
class DriveMechanics;
class MediaDriveService;
}

namespace RetroMediaLab::Emulation {
class AudioOutput final {
public:
    static QList<SettingsChoice> deviceChoices();
    void pump(IEmulatedMachine& machine, bool audible, const ModuleAudioOutputSelection& settings);
    void stop(IEmulatedMachine& machine) noexcept;

private:
    void append(const AudioChunk&, int latencyMs, const QString& deviceId);
    void clear() noexcept;
    std::optional<AudioOutputBufferStatus> status() const;

    std::unique_ptr<QAudioSink> sink_;
    QIODevice* device_ = nullptr;
    QByteArray pending_;
    int latencyMs_ = 0;
    int coreMinimumLatencyMs_ = 0;
    QString deviceId_;
    std::optional<QAudioFormat> requestedFormat_;
};

class MechanicalAudioOutput final {
public:
    explicit MechanicalAudioOutput(const MediaEngine::MediaDriveService&);
    ~MechanicalAudioOutput();

private:
    using Drive = MediaEngine::DriveMechanics;
    struct Sources;
    struct Rendered {
        QByteArray pcm;
        std::vector<std::weak_ptr<Drive>> failed;
    };
    static Rendered mix(const std::vector<std::shared_ptr<Drive>>&, int rate, int frames, int channels);
    void pump() noexcept;
    void refill();
    bool openOutput();
    void finish();

    std::shared_ptr<Sources> sources_;
    QTimer timer_;
    QElapsedTimer drainTime_;
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* device_ = nullptr;
    QByteArray pending_;
    std::future<Rendered> rendering_;
};
}
