#include "VfdProcessing.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr std::array<LinearColor, 4> Tint{{
    {.18f, .78f, 1.f}, {.12f, 1.f, .28f}, {1.f, .48f, .04f}, {1.f, .08f, .03f}
}};
constexpr LinearColor GlassBase{.025f, .055f, .065f};
constexpr LinearColor GlassResponse{.08f, .12f, .13f};

float amount(int value) { return float(value) / VfdProfile::MaximumIntensity; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float smooth(float t) { return t * t * (3.f - 2.f * t); }
float luma(const LinearColor& color) { return color[0] * .2126f + color[1] * .7152f + color[2] * .0722f; }

float cellOffset(int pixel, int output, int source) {
    const float position = (pixel + .5f) * source / output;
    return position - std::floor(position) - .5f;
}

struct HaloSize {
    QSize inner, outer;
};

HaloSize haloSize(QSize output, QSize source, int setting) {
    const float radius = 1.f + amount(setting) * 5.f;
    const int x = std::max(1, int(std::nearbyint(radius * output.width() / source.width())));
    const int y = std::max(1, int(std::nearbyint(radius * output.height() / source.height())));
    return {{std::max(1, x / 2), std::max(1, y / 2)}, {x, y}};
}

float sampleRing(const std::vector<float>& emission, QSize size, int x, int y, QSize radius) {
    const auto at = [&](int dx, int dy) {
        const int sx = std::clamp(x + dx, 0, size.width() - 1), sy = std::clamp(y + dy, 0, size.height() - 1);
        return emission[size_t(sy) * size.width() + sx];
    };
    const int dx = radius.width(), dy = radius.height();
    return (at(-dx, -dy) + at(0, -dy) + at(dx, -dy) + at(-dx, 0) +
        at(dx, 0) + at(-dx, dy) + at(0, dy) + at(dx, dy)) / 8.f;
}
}

VfdProfile normalized(VfdProfile value) {
    for (const auto field : {&VfdProfile::phosphorIntensity, &VfdProfile::emissionThreshold, &VfdProfile::glassDarkening,
            &VfdProfile::cellSize, &VfdProfile::cellGap, &VfdProfile::haloIntensity, &VfdProfile::haloRadius})
        value.*field = std::clamp(value.*field, 0, VfdProfile::MaximumIntensity);
    value.persistenceMilliseconds = std::clamp(value.persistenceMilliseconds, 0, VfdProfile::MaximumPersistenceMilliseconds);
    return value;
}

void applyVfd(std::vector<LinearColor>& colors, QSize output, QSize source, const VfdProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const float threshold = amount(profile.emissionThreshold) * .85f;
    const float radius = (.20f + amount(profile.cellSize) * .28f) * (1.f - amount(profile.cellGap) * .72f);
    const float phosphor = amount(profile.phosphorIntensity) * 1.8f;
    const float haloGain = amount(profile.haloIntensity) * .9f;
    const float glass = amount(profile.glassDarkening);
    const auto tint = Tint[size_t(profile.color)];
    std::vector<float> emission(colors.size());
    for (int y = 0; y < output.height(); ++y) {
        const float dy = cellOffset(y, output.height(), source.height());
        for (int x = 0; x < output.width(); ++x) {
            const size_t i = size_t(y) * output.width() + x;
            const float light = luma(colors[i]);
            float value = smooth(unit((light - threshold) / .08f));
            if (profile.structure == VfdStructure::DotMatrix) {
                const float dx = cellOffset(x, output.width(), source.width());
                const float distance = std::sqrt(dx * dx + dy * dy);
                value *= smooth(unit((radius + .035f - distance) / .035f));
            }
            emission[i] = unit(value * phosphor);
            for (size_t c = 0; c < colors[i].size(); ++c)
                colors[i][c] = colors[i][c] * .28f * (1.f - glass) + (GlassBase[c] + light * GlassResponse[c]) * glass;
        }
    }
    const auto radii = haloSize(output, source, profile.haloRadius);
    for (int y = 0; y < output.height(); ++y)
        for (int x = 0; x < output.width(); ++x) {
            const size_t i = size_t(y) * output.width() + x;
            const float halo = haloGain > 0.f ? unit((sampleRing(emission, output, x, y, radii.inner) * .65f +
                sampleRing(emission, output, x, y, radii.outer) * .35f) * haloGain) : 0.f;
            const float light = unit(emission[i] + halo);
            for (size_t c = 0; c < colors[i].size(); ++c) colors[i][c] = unit(colors[i][c] + light * tint[c]);
        }
}

void applyVfdPersistence(std::vector<LinearColor>& colors, QSize size, std::chrono::nanoseconds timestamp, int milliseconds,
        VfdHistory& history, bool advance) {
    if (!milliseconds) return;
    if (history.size == size && history.colors.size() == colors.size() && timestamp >= history.timestamp) {
        if (!advance && timestamp == history.timestamp) {
            colors = history.colors;
            return;
        }
        const auto elapsed = std::chrono::duration<double, std::milli>(timestamp - history.timestamp).count();
        const auto retention = std::exp(-std::max(.001, elapsed) / std::max(1, milliseconds));
        for (size_t i = 0; i < colors.size(); ++i)
            for (size_t c = 0; c < colors[i].size(); ++c)
                colors[i][c] = std::max(colors[i][c], float(history.colors[i][c] * retention));
    }
    if (advance) history = {size, timestamp, colors};
}
}
