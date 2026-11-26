#pragma once

#include "SoftwareProcessing.h"
#include <QObject>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {

class SoftwareFrameWorker {
public:
    struct Work {
        QImage source;
        QSize sourceSize;
        QSize outputSize;
        Profile profile;
        SignalSimulation signal;
        std::shared_ptr<const Emulation::VideoFrame> frame;
        bool advance;
    };

    using Present = std::function<void(QImage, QSize, double)>;

    explicit SoftwareFrameWorker(Present present)
        : present_(std::move(present)),
          thread_([this](std::stop_token stop) { run(stop); }) {}

    void submit(Work work) {
        {
            std::lock_guard lock(mutex_);
            const bool advance = std::exchange(advancePending_, false);
            work.advance |= advance || (pending_ && pending_->advance);
            pending_ = std::move(work);
        }
        wake_.notify_one();
    }

    void invalidate(bool resetHistory) {
        std::lock_guard lock(mutex_);
        advancePending_ = !resetHistory && (advancePending_ || (pending_ && pending_->advance));
        pending_.reset();
        ready_.reset();
        ++generation_;
        resetHistory_ |= resetHistory;
    }

private:
    struct Result {
        QImage image;
        QSize sourceSize;
        double aspectRatio;
    };

    void run(std::stop_token stop) {
        DisplayHistory history;
        while (!stop.stop_requested()) {
            std::optional<Work> next;
            quint64 generation;
            bool reset;
            {
                std::unique_lock lock(mutex_);
                if (!wake_.wait(lock, stop, [this] { return pending_.has_value(); }) || stop.stop_requested())
                    return;
                next = std::exchange(pending_, std::nullopt);
                generation = generation_;
                reset = std::exchange(resetHistory_, false);
            }
            auto& work = *next;
            if (reset) history = {};
            QImage image;
            try {
                image = process(work.source, work.profile, work.signal, *work.frame, work.outputSize, history, work.advance);
            } catch (...) {
                history = {};
                continue;
            }
            if (image.isNull() || stop.stop_requested()) continue;
            std::lock_guard lock(mutex_);
            if (generation != generation_) continue;
            ready_ = Result{std::move(image), work.sourceSize, frameAspectRatio(*work.frame, work.sourceSize)};
            if (deliveryQueued_) continue;
            deliveryQueued_ = QMetaObject::invokeMethod(&deliveryContext_, [this] { deliver(); }, Qt::QueuedConnection);
        }
    }

    void deliver() {
        std::optional<Result> result;
        {
            std::lock_guard lock(mutex_);
            result = std::exchange(ready_, std::nullopt);
            deliveryQueued_ = false;
        }
        if (result) present_(std::move(result->image), result->sourceSize, result->aspectRatio);
    }

    QObject deliveryContext_;
    Present present_;
    std::mutex mutex_;
    std::condition_variable_any wake_;
    std::optional<Work> pending_;
    std::optional<Result> ready_;
    quint64 generation_ = 0;
    bool resetHistory_ = false;
    bool advancePending_ = false;
    bool deliveryQueued_ = false;
    std::jthread thread_;
};

} // namespace RetroMediaLab::VideoPresentation::SoftwareProcessing
