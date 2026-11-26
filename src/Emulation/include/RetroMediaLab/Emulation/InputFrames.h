#pragma once

#include "Host.h"
#include <mutex>

namespace RetroMediaLab::Emulation {

// UI submissions retain the last absolute state and accumulate relative movement until consumed.
class RETROMEDIALAB_EMULATION_API InputAccumulator final {
public:
    void update(InputSnapshot);
    InputSnapshot consume();
    void reset();

private:
    std::mutex lock_;
    InputSnapshot pending_;
};

// Poll freezes a frame for cores that query input repeatedly during a frame.
class RETROMEDIALAB_EMULATION_API InputFrames final {
public:
    void update(InputSnapshot);
    void poll();
    InputSnapshot polled() const;
    void reset();

private:
    mutable std::mutex lock_;
    InputSnapshot pending_, polled_;
};
}
