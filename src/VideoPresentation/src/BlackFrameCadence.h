#pragma once

#include <cmath>
#include <cstdint>
#include <optional>

namespace RetroMediaLab::VideoPresentation {

class BlackFrameCadence {
public:
    static bool available(double hostHz, double machineHz) noexcept {
        return std::isfinite(hostHz) && std::isfinite(machineHz) && machineHz > 0 && hostHz > machineHz;
    }

    bool shouldBlank(std::uint64_t refresh, bool newImage) noexcept {
        if (!lastRefresh_ || refresh < *lastRefresh_ || newImage) {
            lastRefresh_ = imageRefresh_ = refresh;
            return false;
        }
        const auto last = *lastRefresh_;
        lastRefresh_ = refresh;
        return refresh > last && refresh - last == 1 && refresh - imageRefresh_ == 1;
    }

private:
    std::optional<std::uint64_t> lastRefresh_;
    std::uint64_t imageRefresh_ = 0;
};

}
