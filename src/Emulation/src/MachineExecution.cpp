#include <RetroMediaLab/Emulation/MachineExecution.h>
#include <RetroMediaLab/Emulation/Sdk.h>

#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <initializer_list>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <system_error>
#endif

namespace RetroMediaLab::Emulation {
namespace {
constexpr double MinimumFramesPerSecond = 1.0;
constexpr auto MaximumStateArraySize = static_cast<std::size_t>(std::numeric_limits<qint32>::max());
constexpr double MaximumFramesPerSecond = 1000.0;
constexpr auto ExecutionErrorKey = "Emulation.Error.EmulatorRejected";
#ifdef _WIN32
constexpr DWORD HighResolution = 0x00000002;
constexpr DWORD TimerAccess = TIMER_MODIFY_STATE | SYNCHRONIZE;
using WaitTicks = std::chrono::duration<LONGLONG, std::ratio<1, 10'000'000>>;

struct CloseHandleOnExit {
    void operator()(HANDLE handle) const noexcept { CloseHandle(handle); }
};
using OwnedHandle = std::unique_ptr<void, CloseHandleOnExit>;

[[noreturn]] void throwTimerError() {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), ExecutionErrorKey);
}

OwnedHandle createTimer() {
    OwnedHandle timer(CreateWaitableTimerExW(nullptr, nullptr, HighResolution, TimerAccess));
    if (!timer) timer.reset(CreateWaitableTimerExW(nullptr, nullptr, 0, TimerAccess));
    if (!timer) throwTimerError();
    return timer;
}
#endif
}

struct FrameTimer::State {
#ifdef _WIN32
    OwnedHandle timer = createTimer();
    OwnedHandle stopEvent{CreateEventW(nullptr, TRUE, FALSE, nullptr)};
    State() { if (!stopEvent) throwTimerError(); }
#else
    std::mutex mutex;
    std::condition_variable_any wait;
#endif
};

FrameTimer::FrameTimer() : state_(std::make_unique<State>()) {}
FrameTimer::~FrameTimer() = default;

bool FrameTimer::waitUntil(Clock::time_point deadline, std::stop_token token) {
    if (token.stop_requested()) return false;
    const auto remaining = deadline - Clock::now();
    if (remaining <= Clock::duration::zero()) return true;
#ifdef _WIN32
    const auto stopEvent = state_->stopEvent.get();
    if (!ResetEvent(stopEvent)) throwTimerError();
    const std::stop_callback cancel(token, [stopEvent] { SetEvent(stopEvent); });
    LARGE_INTEGER due{};
    due.QuadPart = -std::chrono::ceil<WaitTicks>(remaining).count();
    if (!SetWaitableTimer(state_->timer.get(), &due, 0, nullptr, nullptr, FALSE)) throwTimerError();
    const HANDLE handles[]{stopEvent, state_->timer.get()};
    const auto result = WaitForMultipleObjects(static_cast<DWORD>(std::size(handles)), handles, FALSE, INFINITE);
    if (result == WAIT_FAILED) throwTimerError();
    if (result == WAIT_OBJECT_0) {
        CancelWaitableTimer(state_->timer.get());
        return false;
    }
#else
    std::unique_lock lock(state_->mutex);
    state_->wait.wait_until(lock, token, deadline, [] { return false; });
#endif
    return !token.stop_requested();
}

FrameTimer::Clock::duration FrameTimer::framePeriod(double framesPerSecond) {
    if (std::isnan(framesPerSecond)) throw std::invalid_argument(ExecutionErrorKey);
    const auto boundedRate = std::clamp(framesPerSecond, MinimumFramesPerSecond, MaximumFramesPerSecond);
    return std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / boundedRate));
}

bool FrameTimer::waitForNextFrame(Clock::time_point& previousDeadline, double framesPerSecond, std::stop_token token) {
    const auto period = framePeriod(framesPerSecond);
    const auto next = previousDeadline + period;
    if (!waitUntil(next, token)) return false;
    const auto now = Clock::now();
    previousDeadline = now - next >= period ? now : next;
    return true;
}

struct MachineCommandCompletion::State {
    std::mutex mutex;
    std::condition_variable_any completed;
    std::exception_ptr error;
    bool ready = false;

    void finish(std::exception_ptr result = {}) {
        {
            const std::lock_guard lock(mutex);
            error = std::move(result);
            ready = true;
        }
        completed.notify_all();
    }
};

MachineCommandCompletion::MachineCommandCompletion(std::shared_ptr<State> state) : state_(std::move(state)) {}

bool MachineCommandCompletion::wait(std::stop_token token) const {
    std::unique_lock lock(state_->mutex);
    if (!state_->completed.wait(lock, token, [this] { return state_->ready; })) return false;
    if (state_->error) std::rethrow_exception(state_->error);
    return true;
}

struct MachineCommandQueue::State {
    struct Command {
        std::function<void()> action;
        std::shared_ptr<MachineCommandCompletion::State> completion;
    };
    explicit State(std::recursive_mutex& inputGate) : gate(inputGate) {}
    std::recursive_mutex& gate;
    std::condition_variable_any changed;
    std::deque<Command> commands;
    const std::exception_ptr stopped = std::make_exception_ptr(std::runtime_error(ExecutionErrorKey));

    std::optional<Command> take() {
        const std::lock_guard lock(gate);
        if (commands.empty()) return {};
        auto command = std::move(commands.front());
        commands.pop_front();
        return command;
    }
};

MachineCommandQueue::MachineCommandQueue(std::recursive_mutex& gate) : state_(std::make_unique<State>(gate)) {}

MachineCommandQueue::~MachineCommandQueue() {
    while (auto command = state_->take()) command->completion->finish(state_->stopped);
}

MachineCommandCompletion MachineCommandQueue::enqueue(std::function<void()> action,
        const std::function<void()>& validateInsideGate) {
    auto completion = std::make_shared<MachineCommandCompletion::State>();
    {
        const std::lock_guard lock(state_->gate);
        if (validateInsideGate) validateInsideGate();
        state_->commands.push_back({std::move(action), completion});
    }
    state_->changed.notify_one();
    return MachineCommandCompletion(std::move(completion));
}

void MachineCommandQueue::failPending(std::exception_ptr error) {
    if (!error) error = state_->stopped;
    // Take the queue's current contents under the machine's gate. The machine
    // can reject later submissions through validation of its stopped state.
    std::deque<State::Command> pending;
    {
        const std::lock_guard lock(state_->gate);
        pending.swap(state_->commands);
    }
    for (auto& command : pending) command.completion->finish(error);
}

void MachineCommandQueue::run(const std::function<bool()>& isPaused, const std::function<bool()>& runFrame,
        const std::function<bool()>& keepRunning, std::stop_token token, std::chrono::milliseconds pauseWait) {
    runFrames(isPaused, runFrame, keepRunning, token, pauseWait, false);
}

void MachineCommandQueue::runPaced(FrameTimer& timer, FrameTimer::Clock::time_point deadline,
        const std::function<bool()>& isPaused, const std::function<void()>& runFrame,
        const std::function<double()>& framesPerSecond, const std::function<bool()>& keepRunning,
        std::stop_token token, std::chrono::milliseconds pauseWait) {
    runFrames(isPaused, [&] {
        runFrame();
        return keepRunning() && timer.waitForNextFrame(deadline, framesPerSecond(), token);
    }, keepRunning, token, pauseWait, true);
}

void MachineCommandQueue::runFrames(const std::function<bool()>& isPaused, const std::function<bool()>& runFrame,
        const std::function<bool()>& keepRunning, std::stop_token token, std::chrono::milliseconds pauseWait,
        bool rethrowCommandError) {
    while (!token.stop_requested() && keepRunning()) {
        {
            std::unique_lock lock(state_->gate);
            if (isPaused() && state_->commands.empty()) {
                state_->changed.wait_for(lock, token, pauseWait,
                    [&] { return !state_->commands.empty() || !isPaused(); });
            }
        }
        if (token.stop_requested()) break;
        while (auto command = state_->take()) {
            try {
                command->action();
            } catch (...) {
                command->completion->finish(std::current_exception());
                if (rethrowCommandError) throw;
                continue;
            }
            command->completion->finish();
        }
        {
            const std::lock_guard lock(state_->gate);
            if (isPaused()) continue;
        }
        if (!runFrame()) break;
    }
}

void softReset(const QSet<int>& keys, const std::function<void(const InputSnapshot&)>& setInput,
        const std::function<void()>& runFrame, const std::function<void()>& nativeReset) {
    if (keys.isEmpty()) {
        nativeReset();
        return;
    }
    try {
        InputSnapshot input;
        input.keys = keys;
        setInput(input);
        runFrame();
    } catch (...) {
        setInput({});
        throw;
    }
    setInput({});
}

void runCore(const std::function<void()>& initialize, const std::function<void()>& run,
        const std::function<std::function<void()>(std::exception_ptr)>& selectFailureHandler,
        const std::function<void(bool)>& cleanup) {
    bool initialized = false;
    try {
        try {
            initialize();
            initialized = true;
            run();
        } catch (...) {
            const auto failure = std::current_exception();
            std::function<void()> handler;
            try {
                handler = selectFailureHandler(failure);
            } catch (...) {
                // As with a C# exception filter, retain the original failure.
                std::rethrow_exception(failure);
            }
            if (!handler) std::rethrow_exception(failure);
            handler();
        }
    } catch (...) {
        cleanup(initialized);
        throw;
    }
    cleanup(initialized);
}

namespace {
void releaseInOrder(std::initializer_list<const std::function<void()>*> steps) {
    std::exception_ptr failure;
    for (const auto* step : steps) {
        if (!step || !*step) continue;
        try {
            (*step)();
        } catch (...) {
            // An exception from a later cleanup replaces an earlier one.
            failure = std::current_exception();
        }
    }
    if (failure) std::rethrow_exception(failure);
}
}

void cleanupCore(bool gameLoaded, bool initialized, const std::function<void()>& unloadGame,
        const std::function<void()>& deinitialize, const std::function<void()>& disposeCallbacks,
        const std::function<void()>& disposeLibrary) {
    releaseInOrder({gameLoaded ? &unloadGame : nullptr, initialized ? &deinitialize : nullptr,
        &disposeCallbacks, &disposeLibrary});
}

void disposeMachine(const std::function<void()>& stopAndJoin, const std::function<void()>& disposeFirst,
        const std::function<void()>& disposeSecond, const std::function<void()>& disposeThird,
        const std::function<void()>& cleanup) {
    releaseInOrder({&stopAndJoin, &disposeFirst, &disposeSecond, &disposeThird, &cleanup});
}

bool tryDisposeCore(const std::function<void()>& dispose,
        const std::function<void(std::exception_ptr)>& reportFailure) {
    try {
        dispose();
        return true;
    } catch (...) {
        reportFailure(std::current_exception());
        return false;
    }
}

bool isSupportedStateSize(std::size_t size, std::size_t maximumSize) noexcept {
    return size > 0 && size <= maximumSize;
}

QByteArray serializeCoreState(bool supported, std::size_t maximumSize,
        const std::function<std::size_t()>& getSerializedSize,
        const std::function<bool(void*, std::size_t)>& serialize,
        const std::function<std::exception_ptr()>& unavailable,
        const std::function<std::exception_ptr()>& saveFailed) {
    const auto size = getSerializedSize();
    if (!supported || !isSupportedStateSize(size, std::min(maximumSize, MaximumStateArraySize)))
        std::rethrow_exception(unavailable());

    QByteArray state(static_cast<qsizetype>(size), Qt::Uninitialized);
    if (!serialize(state.data(), size)) std::rethrow_exception(saveFailed());
    return state;
}

void unserializeCoreState(bool supported, std::size_t maximumSize, QByteArrayView state,
        const std::function<bool(const void*, std::size_t)>& unserialize,
        const std::function<std::exception_ptr()>& invalidSize,
        const std::function<std::exception_ptr()>& loadFailed) {
    const auto size = static_cast<std::size_t>(state.size());
    if (!supported || !isSupportedStateSize(size, std::min(maximumSize, MaximumStateArraySize)))
        std::rethrow_exception(invalidSize());

    const QByteArray buffer(state.data(), state.size());
    if (!unserialize(buffer.constData(), size)) std::rethrow_exception(loadFailed());
}

}
