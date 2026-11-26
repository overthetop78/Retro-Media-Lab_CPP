#pragma once

#include "EmulationExport.h"
#include <QByteArray>
#include <QByteArrayView>
#include <QSet>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>

namespace RetroMediaLab::Emulation {

struct InputSnapshot;

// Owned by the machine's execution thread and destroyed after its last wait.
class RETROMEDIALAB_EMULATION_API FrameTimer final {
public:
    using Clock = std::chrono::steady_clock;

    FrameTimer();
    ~FrameTimer();
    FrameTimer(const FrameTimer&) = delete;
    FrameTimer& operator=(const FrameTimer&) = delete;

    // Returns false when cancellation interrupts the deadline.
    bool waitUntil(Clock::time_point deadline, std::stop_token = {});
    // Keeps the absolute schedule until the delay reaches a full frame.
    bool waitForNextFrame(Clock::time_point& previousDeadline, double framesPerSecond, std::stop_token = {});
    static Clock::duration framePeriod(double framesPerSecond);

private:
    struct State;
    std::unique_ptr<State> state_;
};

class RETROMEDIALAB_EMULATION_API MachineCommandCompletion final {
public:
    // A cancelled wait returns false but leaves the command queued. An executed
    // command's exception is rethrown here on the waiting thread.
    bool wait(std::stop_token = {}) const;

private:
    friend class MachineCommandQueue;
    struct State;
    explicit MachineCommandCompletion(std::shared_ptr<State>);
    std::shared_ptr<State> state_;
};

// The machine owns the gate and keeps it alive through queue destruction.
// The machine thread is the sole caller of run() and runPaced().
class RETROMEDIALAB_EMULATION_API MachineCommandQueue final {
public:
    explicit MachineCommandQueue(std::recursive_mutex& gate);
    ~MachineCommandQueue();
    MachineCommandQueue(const MachineCommandQueue&) = delete;
    MachineCommandQueue& operator=(const MachineCommandQueue&) = delete;

    MachineCommandCompletion enqueue(std::function<void()> action,
        const std::function<void()>& validateInsideGate = {});
    // The machine calls this during shutdown with the cause of termination.
    void failPending(std::exception_ptr = {});

    // isPaused is evaluated under the machine gate. Actions and frames run
    // outside the gate. In the paced variant a failed command also stops the loop.
    void run(const std::function<bool()>& isPaused, const std::function<bool()>& runFrame,
        const std::function<bool()>& keepRunning, std::stop_token, std::chrono::milliseconds pauseWait);
    void runPaced(FrameTimer&, FrameTimer::Clock::time_point firstDeadline,
        const std::function<bool()>& isPaused, const std::function<void()>& runFrame,
        const std::function<double()>& framesPerSecond, const std::function<bool()>& keepRunning,
        std::stop_token, std::chrono::milliseconds pauseWait);

private:
    void runFrames(const std::function<bool()>& isPaused, const std::function<bool()>& runFrame,
        const std::function<bool()>& keepRunning, std::stop_token, std::chrono::milliseconds pauseWait,
        bool rethrowCommandError);
    struct State;
    std::unique_ptr<State> state_;
};

// Called on the machine thread. With keyboard bindings, the native reset is
// replaced by one frame with those keys held, followed by an empty snapshot.
RETROMEDIALAB_EMULATION_API void softReset(const QSet<int>& keys,
    const std::function<void(const InputSnapshot&)>& setInput,
    const std::function<void()>& runFrame, const std::function<void()>& nativeReset);

// The selector returns an empty callback when the failure should propagate.
RETROMEDIALAB_EMULATION_API void runCore(const std::function<void()>& initialize,
    const std::function<void()>& run,
    const std::function<std::function<void()>(std::exception_ptr)>& selectFailureHandler,
    const std::function<void(bool initialized)>& cleanup);

RETROMEDIALAB_EMULATION_API void cleanupCore(bool gameLoaded, bool initialized,
    const std::function<void()>& unloadGame, const std::function<void()>& deinitialize,
    const std::function<void()>& disposeCallbacks, const std::function<void()>& disposeLibrary);

// stopAndJoin waits for the machine thread before its resources are released.
RETROMEDIALAB_EMULATION_API void disposeMachine(const std::function<void()>& stopAndJoin,
    const std::function<void()>& disposeFirst, const std::function<void()>& disposeSecond,
    const std::function<void()>& disposeThird, const std::function<void()>& cleanup);

RETROMEDIALAB_EMULATION_API bool tryDisposeCore(const std::function<void()>& dispose,
    const std::function<void(std::exception_ptr)>& reportFailure);

RETROMEDIALAB_EMULATION_API bool isSupportedStateSize(std::size_t size, std::size_t maximumSize) noexcept;

// The core borrows the native buffer for the duration of the callback.
RETROMEDIALAB_EMULATION_API QByteArray serializeCoreState(bool supported, std::size_t maximumSize,
    const std::function<std::size_t()>& getSerializedSize,
    const std::function<bool(void*, std::size_t)>& serialize,
    const std::function<std::exception_ptr()>& unavailable,
    const std::function<std::exception_ptr()>& saveFailed);

RETROMEDIALAB_EMULATION_API void unserializeCoreState(bool supported, std::size_t maximumSize,
    QByteArrayView state, const std::function<bool(const void*, std::size_t)>& unserialize,
    const std::function<std::exception_ptr()>& invalidSize,
    const std::function<std::exception_ptr()>& loadFailed);

// The caller owns the sequence state and calls this on the machine thread.
// The publishers borrow the frame and the audio block for the call.
template<class Machine, class LatestVideo, class TakeAudio, class PublishVideo, class PublishAudio>
void publishOutputs(Machine& machine, quint64& videoSequence, LatestVideo&& latestVideo,
    TakeAudio&& takeAudio, PublishVideo&& publishVideo, PublishAudio&& publishAudio) {
    if (const auto video = latestVideo(machine); video && video->sequence != videoSequence) {
        videoSequence = video->sequence;
        publishVideo(machine, *video);
    }
    while (const auto audio = takeAudio(machine)) {
        publishAudio(machine, *audio);
    }
}

}
