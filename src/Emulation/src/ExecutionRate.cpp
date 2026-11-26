#include <RetroMediaLab/Emulation/ExecutionRate.h>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto MeasurementInterval = std::chrono::seconds(1);
constexpr double HertzPerMegahertz = 1'000'000;
}

void ExecutionRateWindow::reset() noexcept {
    previous_.reset();
    frameCount_ = 0;
    fps_ = 0;
    rate_.reset();
}

void ExecutionRateWindow::observe(std::uint64_t sequence, std::chrono::nanoseconds emulatedTime,
        Clock::time_point now, std::optional<std::uint64_t> processorClockHz) {
    if (previous_ && previous_->sequence == sequence && previous_->emulatedTime == emulatedTime) return;
    if (!previous_ || sequence <= previous_->sequence || emulatedTime < previous_->emulatedTime) {
        reset();
        windowStart_ = now;
        emulatedStart_ = emulatedTime;
    } else {
        ++frameCount_;
    }
    previous_ = Sample{sequence, emulatedTime};
    const auto elapsed = now - windowStart_;
    if (elapsed < MeasurementInterval) return;
    const auto realSeconds = std::chrono::duration<double>(elapsed).count();
    fps_ = static_cast<double>(frameCount_) / realSeconds;
    rate_.reset();
    if (emulatedTime > emulatedStart_) {
        const auto emulatedSeconds = std::chrono::duration<double>(emulatedTime).count()
            - std::chrono::duration<double>(emulatedStart_).count();
        const auto ratio = emulatedSeconds / realSeconds;
        rate_ = ExecutionRate{ratio, std::nullopt};
        if (processorClockHz.value_or(0) > 0)
            rate_->effectiveMegahertz = static_cast<double>(*processorClockHz) * ratio / HertzPerMegahertz;
    }
    windowStart_ = now;
    emulatedStart_ = emulatedTime;
    frameCount_ = 0;
}

}
