#pragma once

#include "AnalogSignal.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
inline float matrixLuminance(const LinearColor& c) { return c[0] * .2126f + c[1] * .7152f + c[2] * .0722f; }

inline float matrixSmoothStep(float start, float end, float value) {
    const float t = std::clamp((value - start) / (end - start), 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

inline LinearColor sampleMatrixCell(const std::vector<LinearColor>& source, QSize input, QSize grid, int x, int y) {
    const float offsetX = grid.width() < input.width() ? .25f : 0.f;
    const float offsetY = grid.height() < input.height() ? .25f : 0.f;
    const auto sample = [&](float column, float row) -> const LinearColor& {
        const int sx = std::min(input.width() - 1, int(column / grid.width() * input.width()));
        const int sy = std::min(input.height() - 1, int(row / grid.height() * input.height()));
        return source[size_t(sy) * input.width() + sx];
    };
    const float cx = x + .5f, cy = y + .5f;
    auto color = sample(cx, cy);
    if (offsetX || offsetY) {
        const auto& topLeft = sample(cx - offsetX, cy - offsetY);
        const auto& topRight = sample(cx + offsetX, cy - offsetY);
        const auto& bottomLeft = sample(cx - offsetX, cy + offsetY);
        const auto& bottomRight = sample(cx + offsetX, cy + offsetY);
        for (size_t channel = 0; channel < color.size(); ++channel)
            color[channel] = (4.f * color[channel] + topLeft[channel] + topRight[channel] +
                bottomLeft[channel] + bottomRight[channel]) * .125f;
    }
    return color;
}

}
