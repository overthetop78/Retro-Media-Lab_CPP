#pragma once

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
inline float linear(float value) {
    return value <= .04045f ? value / 12.92f : std::pow((value + .055f) / 1.055f, 2.4f);
}

inline float srgb(float value) {
    value = std::clamp(value, 0.f, 1.f);
    return value <= .0031308f ? 12.92f * value : 1.055f * std::pow(value, 1.f / 2.4f) - .055f;
}
}
