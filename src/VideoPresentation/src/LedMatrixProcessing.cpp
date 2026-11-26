#include "LedMatrixProcessing.h"
#include "MatrixCells.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr std::array<LinearColor, 6> Tints{{
    {1.f, 1.f, 1.f}, {1.f, .03f, .01f}, {.03f, 1.f, .08f}, {1.f, .42f, .02f}, {.03f, .25f, 1.f}, {1.f, 1.f, 1.f}
}};

float pct(int value) { return float(value) / LedMatrixProfile::MaximumIntensity; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }

}

LedMatrixProfile normalized(LedMatrixProfile settings) {
    for (const auto field : {&LedMatrixProfile::cellSize, &LedMatrixProfile::cellGap, &LedMatrixProfile::diffusion,
            &LedMatrixProfile::brightness, &LedMatrixProfile::haloRadius, &LedMatrixProfile::blackDepth})
        settings.*field = std::clamp(settings.*field, 0, LedMatrixProfile::MaximumIntensity);
    return settings;
}

void applyLedMatrix(std::vector<LinearColor>& colors, QSize output, QSize source, const LedMatrixProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const int pitch = 1 + int(std::nearbyint(pct(profile.cellSize) * 7.f));
    MatrixCells cells(colors, output, source, pitch);
    const auto tint = Tints[size_t(profile.color)];
    const float brightness = pct(profile.brightness) * 1.35f;
    for (auto& cell : cells.averages()) {
        if (profile.color != LedMatrixColor::Rgb) {
            const float luminance = matrixLuminance(cell);
            for (size_t channel = 0; channel < tint.size(); ++channel) cell[channel] = luminance * tint[channel];
        }
        for (auto& value : cell) value *= brightness;
    }
    const float panel = (1.f - pct(profile.blackDepth)) * .08f;
    const float radius = .5f * (1.f - pct(profile.cellGap) * .86f);
    const float haloFactor = 4.f / (.04f + pct(profile.haloRadius) * .9f);
    const float haloIntensity = pct(profile.diffusion);
    for (int y = 0; y < output.height(); ++y) {
        const auto row = cells.row(y);
        for (int x = 0; x < output.width(); ++x) {
            const auto column = cells.column(x);
            const float distance = profile.shape == LedMatrixShape::Round
                ? std::sqrt(column.offset * column.offset + row.offset * row.offset)
                : std::max(std::abs(column.offset), std::abs(row.offset));
            const float core = cells.core(distance, radius);
            const float halo = haloIntensity > 0.f ? std::exp(-std::max(0.f, distance - radius) * haloFactor) *
                (1.f - core) * haloIntensity : 0.f;
            const auto& emission = cells.averages()[cells.index(column, row)];
            auto& color = colors[size_t(y) * output.width() + x];
            for (size_t channel = 0; channel < color.size(); ++channel)
                color[channel] = unit(panel + emission[channel] * (core + halo));
        }
    }
}
}
