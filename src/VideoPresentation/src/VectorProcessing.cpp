#include "VectorProcessing.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr std::array<LinearColor, 5> PhosphorTint{{
    {1.f, 1.f, 1.f}, {.18f, 1.f, .24f}, {1.f, .46f, .035f}, {1.f, 1.f, 1.f}, {.78f, .80f, .77f}
}};

float level(int value) { return float(value) / VectorProfile::MaximumIntensity; }
float luma(const LinearColor& c) { return c[0] * .2126f + c[1] * .7152f + c[2] * .0722f; }
size_t index(QSize size, int x, int y) { return size_t(y) * size.width() + x; }

float sample(const std::vector<float>& values, QSize size, int x, int y) {
    return values[index(size, std::clamp(x, 0, size.width() - 1), std::clamp(y, 0, size.height() - 1))];
}

std::vector<float> lineEmission(const std::vector<LinearColor>& colors, QSize size, int threshold) {
    std::vector<float> luminance(colors.size()), result(colors.size());
    std::transform(colors.begin(), colors.end(), luminance.begin(), luma);
    const float low = level(threshold), high = std::min(1.f, low + .1f);
    for (int y = 0; y < size.height(); ++y)
        for (int x = 0; x < size.width(); ++x) {
            const auto at = [&](int dx, int dy) { return sample(luminance, size, x + dx, y + dy); };
            const float tl = at(-1, -1), tr = at(1, -1), bl = at(-1, 1), br = at(1, 1);
            const float gx = tr + 2.f * at(1, 0) + br - tl - 2.f * at(-1, 0) - bl;
            const float gy = bl + 2.f * at(0, 1) + br - tl - 2.f * at(0, -1) - tr;
            const float magnitude = std::clamp(std::sqrt(gx * gx + gy * gy) / 4.f, 0.f, 1.f);
            const float t = high > low ? std::clamp((magnitude - low) / (high - low), 0.f, 1.f)
                : float(magnitude >= high);
            result[index(size, x, y)] = t * t * (3.f - 2.f * t);
        }
    return result;
}

void widen(std::vector<float>& emission, QSize size, int setting) {
    if (!setting) return;
    const auto source = emission;
    const float strength = level(setting);
    const int radius = 1 + int(std::floor(strength * 2.99f));
    for (int y = 0; y < size.height(); ++y)
        for (int x = 0; x < size.width(); ++x) {
            auto& value = emission[index(size, x, y)];
            float peak = value;
            for (int dy = -radius; dy <= radius; ++dy)
                for (int dx = -radius; dx <= radius; ++dx)
                    if (dx * dx + dy * dy <= radius * radius)
                        peak = std::max(peak, sample(source, size, x + dx, y + dy));
            value = std::lerp(value, peak, strength);
        }
}

std::vector<float> average(const std::vector<float>& source, QSize size, int radius) {
    std::vector<float> horizontal(source.size()), result(source.size());
    const int side = 2 * radius + 1;
    const float scale = 1.f / (side * side);
    for (int y = 0; y < size.height(); ++y) {
        float rolling = 0;
        for (int dx = -radius; dx <= radius; ++dx) rolling += sample(source, size, dx, y);
        for (int x = 0; x < size.width(); ++x) {
            horizontal[index(size, x, y)] = rolling;
            rolling += sample(source, size, x + radius + 1, y) - sample(source, size, x - radius, y);
        }
    }
    for (int x = 0; x < size.width(); ++x) {
        float rolling = 0;
        for (int dy = -radius; dy <= radius; ++dy) rolling += sample(horizontal, size, x, dy);
        for (int y = 0; y < size.height(); ++y) {
            result[index(size, x, y)] = std::clamp(rolling * scale, 0.f, 1.f);
            rolling += sample(horizontal, size, x, y + radius + 1) - sample(horizontal, size, x, y - radius);
        }
    }
    return result;
}
}

VectorProfile normalized(VectorProfile profile) {
    for (const auto field : {&VectorProfile::lineThreshold, &VectorProfile::lineIntensity, &VectorProfile::beamWidth,
            &VectorProfile::beamFocus, &VectorProfile::haloIntensity, &VectorProfile::haloRadius, &VectorProfile::persistenceIntensity})
        profile.*field = std::clamp(profile.*field, 0, VectorProfile::MaximumIntensity);
    if (size_t(profile.phosphorColor) >= PhosphorTint.size()) profile.phosphorColor = VectorPhosphor::Color;
    return profile;
}

void applyVector(std::vector<LinearColor>& colors, QSize size, const VectorProfile& profile) {
    if (!profile.lineIntensity || colors.empty() || size.isEmpty()) return;
    auto emission = lineEmission(colors, size, profile.lineThreshold);
    widen(emission, size, profile.beamWidth);
    if (profile.beamFocus < VectorProfile::MaximumIntensity) {
        const auto spread = average(emission, size, 1);
        const float blur = (1.f - level(profile.beamFocus)) * .65f;
        for (size_t i = 0; i < emission.size(); ++i) emission[i] = std::lerp(emission[i], spread[i], blur);
    }
    const auto halo = profile.haloIntensity ? average(emission, size, 1 + int(std::nearbyint(level(profile.haloRadius) * 5.f)))
        : std::vector<float>{};
    const float strength = level(profile.lineIntensity), haloStrength = strength * level(profile.haloIntensity) * .5f;
    const auto tint = PhosphorTint[size_t(profile.phosphorColor)];
    for (size_t i = 0; i < colors.size(); ++i) {
        auto& color = colors[i];
        const float diffuse = halo.empty() ? 0.f : halo[i] * haloStrength;
        for (auto& value : color) {
            value = std::lerp(value, 1.f, emission[i] * strength);
            if (!halo.empty()) value = std::clamp(value + diffuse, 0.f, 1.f);
        }
        if (profile.phosphorColor == VectorPhosphor::Color) continue;
        const float luminance = luma(color);
        for (size_t c = 0; c < color.size(); ++c) color[c] = luminance * tint[c];
    }
}

void applyVectorPersistence(std::vector<LinearColor>& colors, QSize size, quint64 sequence, int intensity,
        VectorHistory& history, bool advance) {
    if (!intensity) return;
    if (history.size == size && history.colors.size() == colors.size() && sequence >= history.sequence) {
        if (!advance && sequence == history.sequence) {
            colors = history.colors;
            return;
        }
        const float retention = level(intensity);
        for (size_t i = 0; i < colors.size(); ++i)
            for (size_t c = 0; c < colors[i].size(); ++c)
                colors[i][c] = std::max(colors[i][c], history.colors[i][c] * retention);
    }
    if (advance) history = {size, sequence, colors};
}
}
