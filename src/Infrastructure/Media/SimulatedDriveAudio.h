#pragma once
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include <RetroMediaLab/MediaEngine/MediaKind.h>
#include <QByteArray>
#include <QTimer>
#include <future>
#include <memory>

class QAudioSink;
class QIODevice;

namespace RetroMediaLab::Infrastructure {
class SimulatedDriveAudio final {
public:
    SimulatedDriveAudio();
    ~SimulatedDriveAudio();
    void start(MediaEngine::MediaKind);
    void position(MediaEngine::DrivePosition value) { position_ = value; }
    void finish();
    void cancel();

private:
    void pump();
    std::shared_ptr<MediaEngine::DriveMechanics> drive_;
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* output_ = nullptr;
    QTimer timer_;
    std::future<QByteArray> pending_;
    QByteArray buffer_;
    MediaEngine::DrivePosition position_;
    unsigned request_ = 0;
    unsigned pendingRequest_ = 0;
    bool active_ = false;
    bool tape_ = false;
    bool stopped_ = false;
};
}
