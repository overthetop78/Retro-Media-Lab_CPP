#include "DotMatrixProcessing.h"
#include "MatrixCells.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
struct Palette {
    LinearColor background;
    LinearColor foreground;
};

constexpr std::array<Palette, 7> Palettes{{
    {{.16f, .25f, .075f}, {.018f, .045f, .01f}}, {{.64f, .68f, .62f}, {.035f, .035f, .035f}},
    {{}, {1.f, .42f, .015f}}, {{}, {.32f, .72f, 1.f}}, {{}, {}}, {{}, {1.f, .06f, .025f}}, {{}, {1.f, 1.f, 1.f}}
}};

float amount(int value) { return float(value) / DotMatrixProfile::MaximumIntensity; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }

float distance(float x, float y, DotMatrixShape shape) {
    switch (shape) {
    case DotMatrixShape::Square: return std::max(std::abs(x), std::abs(y));
    case DotMatrixShape::Rectangle: return std::max(.72f * std::abs(x), std::abs(y));
    default: return std::sqrt(x * x + y * y);
    }
}
}

DotMatrixProfile normalized(DotMatrixProfile profile) {
    for (const auto field : {&DotMatrixProfile::dotSize, &DotMatrixProfile::contrast, &DotMatrixProfile::cellSize,
            &DotMatrixProfile::cellGap, &DotMatrixProfile::brightness, &DotMatrixProfile::haloIntensity})
        profile.*field = std::clamp(profile.*field, 0, DotMatrixProfile::MaximumIntensity);
    for (const auto field : {&DotMatrixProfile::responseTimeMilliseconds, &DotMatrixProfile::persistenceMilliseconds})
        profile.*field = std::clamp(profile.*field, 0, DotMatrixProfile::MaximumDurationMilliseconds);
    return profile;
}

void applyDotMatrix(std::vector<LinearColor>& colors, QSize output, QSize source, const DotMatrixProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const int pitch = 1 + int(std::nearbyint(amount(profile.cellSize) * 15.f));
    MatrixCells cells(colors, output, source, pitch);
    const auto palette = Palettes[size_t(profile.palette)];
    const float contrast = .5f + amount(profile.contrast) * 2.5f;
    const float brightness = .05f + amount(profile.brightness) * 1.2f;
    for (auto& cell : cells.averages()) {
        const auto luminance = matrixLuminance(cell);
        const float activation = unit((luminance - .5f) * contrast + .5f) * brightness;
        auto foreground = palette.foreground;
        if (profile.palette == DotMatrixPalette::Rgb)
            for (size_t c = 0; c < foreground.size(); ++c) foreground[c] = unit(cell[c] / std::max(.0001f, luminance));
        for (size_t c = 0; c < cell.size(); ++c) cell[c] = (foreground[c] - palette.background[c]) * activation;
    }
    const float radius = .08f + .42f * amount(profile.dotSize) * (1.f - .72f * amount(profile.cellGap));
    const float haloGain = amount(profile.haloIntensity) * .85f;
    for (int y = 0; y < output.height(); ++y) {
        const auto row = cells.row(y);
        for (int x = 0; x < output.width(); ++x) {
            const auto column = cells.column(x);
            const float d = distance(column.offset, row.offset, profile.shape);
            const float core = cells.core(d, radius);
            const float spatial = core + (haloGain > 0 ? std::exp(-std::max(0.f, d - radius) * 8.f) * haloGain * (1.f - core) : 0.f);
            auto& color = colors[size_t(y) * output.width() + x];
            const auto& cell = cells.averages()[cells.index(column, row)];
            for (size_t c = 0; c < color.size(); ++c) color[c] = unit(palette.background[c] + cell[c] * spatial);
        }
    }
}

void applyDotMatrixTemporal(std::vector<LinearColor>& colors, QSize size, std::chrono::nanoseconds timestamp,
        const DotMatrixProfile& profile, DotMatrixHistory& history, bool advance) {
    if (!profile.responseTimeMilliseconds && !profile.persistenceMilliseconds) return;
    if (history.size == size && history.colors.size() == colors.size() && timestamp >= history.timestamp) {
        if (!advance && history.timestamp == timestamp) {
            colors = history.colors;
            return;
        }
        const double elapsed = std::max(.001, std::chrono::duration<double, std::milli>(timestamp - history.timestamp).count());
        const float response = profile.responseTimeMilliseconds > 0
            ? float(-std::expm1(-elapsed / profile.responseTimeMilliseconds)) : 1.f;
        const float decay = profile.persistenceMilliseconds > 0
            ? std::exp(-float(elapsed) / profile.persistenceMilliseconds) : 0.f;
        const bool reflective = profile.palette == DotMatrixPalette::Green || profile.palette == DotMatrixPalette::Gray;
        const auto background = Palettes[size_t(profile.palette)].background;
        for (size_t i = 0; i < colors.size(); ++i)
            for (size_t c = 0; c < colors[i].size(); ++c) {
                const auto previous = history.colors[i][c];
                auto& value = colors[i][c];
                value = std::lerp(previous, value, response);
                if (profile.persistenceMilliseconds > 0)
                    value = reflective ? std::min(value, background[c] - (background[c] - previous) * decay) :
                        std::max(value, previous * decay);
            }
    }
    if (advance) history = {size, timestamp, colors};
}
}
