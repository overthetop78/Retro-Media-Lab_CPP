#include "SimulatedDriveAudio.h"
#include <RetroMediaLab/MediaEngine/DriveService.h>
#include <QAudioDevice>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>
#include <chrono>
#include <cstring>

namespace RetroMediaLab::Infrastructure {
namespace {
constexpr int PumpMilliseconds = 10;
constexpr int ChunkMilliseconds = 40;
constexpr int BufferChunks = 3;
constexpr int MillisecondsPerSecond = 1000;
constexpr int MonoChannels = 1;
constexpr auto TapeDevice = "Tape";
constexpr auto TapePlay = "Play";
constexpr auto TapeStop = "Stop";

QByteArray render(MediaEngine::DriveMechanics& drive, MediaEngine::DrivePosition position,
                  bool tape, bool active, bool stop, int rate, int frames, int channels) {
    if (active) {
        if (tape) {
            drive.setTapeOperation(TapePlay);
        } else {
            drive.setMotor(true);
            drive.setActivity(true, false);
            drive.seek(position);
        }
    } else if (stop) {
        if (tape) drive.setTapeOperation(TapeStop);
        else drive.setMotor(false);
    }
    const auto samples = drive.renderAudio(rate, frames);
    QByteArray result(static_cast<qsizetype>(samples.size() * channels * sizeof(std::int16_t)), Qt::Uninitialized);
    auto* destination = result.data();
    for (const auto sample : samples) {
        for (int channel = 0; channel < channels; ++channel) {
            std::memcpy(destination, &sample, sizeof(sample));
            destination += sizeof(sample);
        }
    }
    return result;
}
}

SimulatedDriveAudio::SimulatedDriveAudio() {
    timer_.setInterval(PumpMilliseconds);
    QObject::connect(&timer_, &QTimer::timeout, &timer_, [this] { pump(); });
}

SimulatedDriveAudio::~SimulatedDriveAudio() { cancel(); }

void SimulatedDriveAudio::start(MediaEngine::MediaKind kind) {
    cancel();
    try {
        const auto device = QMediaDevices::defaultAudioOutput();
        if (device.isNull()) return;
        auto format = device.preferredFormat();
        format.setSampleFormat(QAudioFormat::Int16);
        auto mono = format;
        mono.setChannelCount(MonoChannels);
        if (device.isFormatSupported(mono)) format = mono;
        if (!device.isFormatSupported(format)) return;

        tape_ = kind == MediaEngine::MediaKind::Tape;
        const MediaEngine::MediaDriveService service;
        drive_ = tape_ ? service.create(TapeDevice) : service.create(kind);
        if (!drive_) return;
        position_ = {};
        active_ = true;
        stopped_ = false;
        sink_ = std::make_unique<QAudioSink>(device, format);
        sink_->setBufferSize(format.bytesForDuration(BufferChunks * ChunkMilliseconds * MillisecondsPerSecond));
        output_ = sink_->start();
        if (!output_) { cancel(); return; }
        timer_.start();
        pump();
    } catch (...) {
        cancel();
    }
}

void SimulatedDriveAudio::finish() {
    active_ = false;
}

void SimulatedDriveAudio::cancel() {
    ++request_;
    timer_.stop();
    active_ = false;
    output_ = nullptr;
    if (sink_) sink_->reset();
    sink_.reset();
    drive_.reset();
    buffer_.clear();
}

void SimulatedDriveAudio::pump() {
    try {
        if (sink_->error() != QAudio::NoError) {
            cancel();
            return;
        }
        if (pending_.valid()) {
            if (pending_.wait_for(std::chrono::seconds::zero()) != std::future_status::ready) return;
            try {
                auto pcm = pending_.get();
                if (pendingRequest_ == request_) buffer_ = std::move(pcm);
            } catch (...) {
                if (pendingRequest_ == request_) { cancel(); return; }
            }
        }
        if (!buffer_.isEmpty()) {
            const auto written = output_->write(buffer_.constData(), buffer_.size());
            if (written < 0) { cancel(); return; }
            buffer_.remove(0, written);
            if (!buffer_.isEmpty()) return;
        }
        if (stopped_ && !drive_->hasPendingAudio()) {
            if (sink_->state() == QAudio::IdleState) cancel();
            return;
        }
        const auto format = sink_->format();
        const auto duration = ChunkMilliseconds * MillisecondsPerSecond;
        if (sink_->bytesFree() < format.bytesForDuration(duration)) return;
        const auto stop = !active_ && !stopped_;
        stopped_ = stopped_ || stop;
        pendingRequest_ = request_;
        pending_ = std::async(std::launch::async,
            [drive=drive_, position=position_, tape=tape_, active=active_, stop, format, duration] {
                return render(*drive, position, tape, active, stop, format.sampleRate(),
                              format.framesForDuration(duration), format.channelCount());
            });
    } catch (...) {
        cancel();
    }
}
}
