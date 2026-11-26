#pragma once

#include <RetroMediaLab/Emulation/Sdk.h>
#include <QAudioSink>

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
}
