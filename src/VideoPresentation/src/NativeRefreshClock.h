#pragma once

#include <cstdint>
#include <functional>
#include <stop_token>

namespace RetroMediaLab::VideoPresentation {

void runNativeRefreshClock(std::uintptr_t window, std::stop_token,
    const std::function<void(std::uint64_t, double)>& present);

}
