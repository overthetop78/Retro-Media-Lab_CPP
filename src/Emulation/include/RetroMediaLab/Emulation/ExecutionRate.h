#pragma once

#include "EmulationExport.h"
#include <chrono>
#include <cstdint>
#include <optional>

namespace RetroMediaLab::Emulation {

class IMachineProcessorClock {
public:
    virtual ~IMachineProcessorClock() = default;
    virtual std::optional<std::uint64_t> processorClockHz() const = 0;
};

struct ExecutionRate {
    double speedRatio = 0;
    std::optional<double> effectiveMegahertz;
};

class RETROMEDIALAB_EMULATION_API ExecutionRateWindow final {
public:
    using Clock = std::chrono::steady_clock;

    void reset() noexcept;
    void observe(std::uint64_t sequence, std::chrono::nanoseconds emulatedTime, Clock::time_point now,
        std::optional<std::uint64_t> processorClockHz);
    double measuredFramesPerSecond() const noexcept { return fps_; }
    const std::optional<ExecutionRate>& rate() const noexcept { return rate_; }

private:
    struct Sample {
        std::uint64_t sequence;
        std::chrono::nanoseconds emulatedTime;
    };
    std::optional<Sample> previous_;
    Clock::time_point windowStart_{};
    std::chrono::nanoseconds emulatedStart_{};
    std::uint64_t frameCount_ = 0;
    double fps_ = 0;
    std::optional<ExecutionRate> rate_;
};

}
