#include <RetroMediaLab/Emulation/InputFrames.h>

#include <algorithm>
#include <limits>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
int saturatingAdd(int first, int second) {
    constexpr auto Minimum = std::numeric_limits<int>::min();
    constexpr auto Maximum = std::numeric_limits<int>::max();
    return static_cast<int>(std::clamp(qint64(first) + second, qint64(Minimum), qint64(Maximum)));
}

void accumulate(InputSnapshot& pending, InputSnapshot next) {
    next.pointerX = saturatingAdd(pending.pointerX, next.pointerX);
    next.pointerY = saturatingAdd(pending.pointerY, next.pointerY);
    next.wheel = saturatingAdd(pending.wheel, next.wheel);
    next.horizontalWheel = saturatingAdd(pending.horizontalWheel, next.horizontalWheel);
    pending = std::move(next);
}

InputSnapshot consumeRelativeInput(InputSnapshot& pending) {
    const InputSnapshot current = pending;
    pending.pointerX = pending.pointerY = pending.wheel = pending.horizontalWheel = 0;
    return current;
}
}

void InputAccumulator::update(InputSnapshot input) {
    const std::lock_guard guard(lock_);
    accumulate(pending_, std::move(input));
}

InputSnapshot InputAccumulator::consume() {
    const std::lock_guard guard(lock_);
    return consumeRelativeInput(pending_);
}

void InputAccumulator::reset() {
    const std::lock_guard guard(lock_);
    pending_ = {};
}

void InputFrames::update(InputSnapshot input) {
    const std::lock_guard guard(lock_);
    accumulate(pending_, std::move(input));
}

void InputFrames::poll() {
    const std::lock_guard guard(lock_);
    polled_ = consumeRelativeInput(pending_);
}

InputSnapshot InputFrames::polled() const {
    const std::lock_guard guard(lock_);
    return polled_;
}

void InputFrames::reset() {
    const std::lock_guard guard(lock_);
    pending_ = {};
    polled_ = {};
}
}
