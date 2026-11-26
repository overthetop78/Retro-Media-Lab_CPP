#include "StylisticProcessing.h"

#include <algorithm>
#include <cmath>
#include <span>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr int BloomRadius = 4;
constexpr float BloomThreshold = .58f;

float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float amount(int setting) { return float(setting) / StylisticEffects::MaximumIntensity; }

void applyVhs(std::vector<LinearColor>& colors, int width, int height, quint64 sequence, float strength) {
    if (strength <= 0 || width < 2) return;
    const auto source = colors;
    const auto luma = [](const auto& color) { return color[0] * .299f + color[1] * .587f + color[2] * .114f; };
    for (int y = 0; y < height; ++y) {
        auto hash = quint32(y) * 1103515245u + quint32(sequence) * 12345u;
        hash ^= hash >> 15;
        const float wobble = std::sin(y * .071f + float(sequence) * .31f) * strength * 4.f;
        const float jitter = (float(hash & 15u) / 7.5f - 1.f) * strength * 2.f;
        const int shift = int(std::nearbyint(wobble + jitter));
        const float dropout = (hash & 127u) > 122u ? strength * .42f : 0.f;
        const float head = y > height * .88f ? std::sin(y * .73f + float(sequence) * .43f) * strength * .16f : 0.f;
        const auto row = size_t(y) * width;
        for (int x = 0; x < width; ++x) {
            const int shifted = std::clamp(x + shift, 0, width - 1);
            const auto& center = source[row + shifted];
            const auto& left = source[row + std::max(0, shifted - 2)];
            const auto& right = source[row + std::min(width - 1, shifted + 2)];
            const float cy = luma(center), ly = luma(left), ry = luma(right);
            const float light = std::lerp(cy, (ly + cy * 2.f + ry) * .25f, strength * .72f);
            auto& target = colors[row + x];
            for (size_t channel = 0; channel < target.size(); ++channel) {
                const float chroma = std::lerp(center[channel] - cy, left[channel] - ly, strength * .68f) *
                    (1.f - strength * .28f);
                target[channel] = unit((light + chroma) * (1.f - dropout) + head);
            }
        }
    }
}

void applyAberration(std::vector<LinearColor>& colors, int width, int height, float strength) {
    if (strength <= 0 || width < 2) return;
    const std::vector<LinearColor> source = colors;
    const float offset = strength * 7.f;
    for (int y = 0; y < height; ++y) {
        const auto row = size_t(y) * width;
        const auto sample = [&](float x, size_t channel) {
            const auto at = std::clamp(x, 0.f, float(width - 1));
            const int left = int(at);
            return std::lerp(source[row + left][channel], source[row + std::min(width - 1, left + 1)][channel], at - left);
        };
        for (int x = 0; x < width; ++x) {
            colors[row + x][0] = sample(x + offset, 0);
            colors[row + x][2] = sample(x - offset, 2);
        }
    }
}

void applyBloom(std::vector<LinearColor>& colors, int width, int height, float strength) {
    if (strength <= 0) return;
    // Keep the clipped 9-by-9 window of FilterBloom with a running sum per column.
    std::vector<LinearColor> highlights(colors.size()), columns(size_t(width), LinearColor{});
    for (size_t index = 0; index < colors.size(); ++index)
        for (size_t channel = 0; channel < colors[index].size(); ++channel)
            highlights[index][channel] = std::max(0.f, colors[index][channel] - BloomThreshold) / (1.f - BloomThreshold);
    const auto addRow = [&](int y, float sign) {
        const auto row = size_t(y) * width;
        for (int x = 0; x < width; ++x)
            for (size_t c = 0; c < columns[x].size(); ++c) columns[x][c] += sign * highlights[row + x][c];
    };
    for (int y = 0; y < std::min(height, BloomRadius); ++y) addRow(y, 1.f);
    for (int y = 0; y < height; ++y) {
        if (y + BloomRadius < height) addRow(y + BloomRadius, 1.f);
        if (y > BloomRadius) addRow(y - BloomRadius - 1, -1.f);
        const int rows = std::min(height - 1, y + BloomRadius) - std::max(0, y - BloomRadius) + 1;
        LinearColor light{};
        for (int x = 0; x < std::min(width, BloomRadius); ++x)
            for (size_t c = 0; c < light.size(); ++c) light[c] += columns[x][c];
        for (int x = 0; x < width; ++x) {
            const int count = std::min(width - 1, x + BloomRadius) - std::max(0, x - BloomRadius) + 1;
            const float gain = strength * .72f / (rows * count);
            auto& color = colors[size_t(y) * width + x];
            for (size_t c = 0; c < light.size(); ++c) {
                if (x + BloomRadius < width) light[c] += columns[x + BloomRadius][c];
                if (x > BloomRadius) light[c] -= columns[x - BloomRadius - 1][c];
                color[c] = unit(color[c] + std::max(0.f, light[c]) * gain);
            }
        }
    }
}

void applySepia(std::span<LinearColor> colors) {
    for (auto& color : colors) {
        const auto [red, green, blue] = color;
        color = {unit((red * .393f + green * .769f + blue * .189f) * 1.08f),
            unit((red * .349f + green * .686f + blue * .168f) * .92f),
            unit((red * .272f + green * .534f + blue * .131f) * .68f)};
    }
}

void applyGrain(std::vector<LinearColor>& colors, int width, int height, quint64 sequence, float strength) {
    if (strength <= 0) return;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            auto hash = (quint32(x) * 1597334677u) ^ (quint32(y) * 3812015801u) ^ (quint32(sequence) * 95868931u);
            hash ^= hash >> 16;
            hash *= 2246822519u;
            auto& color = colors[size_t(y) * width + x];
            const float luminance = color[0] * .2126f + color[1] * .7152f + color[2] * .0722f;
            const float visibility = .22f + .78f * (1.f - std::abs(luminance * 2.f - 1.f));
            const float noise = (float(hash & 2047u) / 1023.5f - 1.f) * strength * .045f * visibility;
            for (auto& value : color) value = unit(value + noise);
        }
}
}

StylisticEffects normalized(StylisticEffects settings) {
    for (const auto field : {&StylisticEffects::grain, &StylisticEffects::vhs, &StylisticEffects::chromaticAberration, &StylisticEffects::bloom})
        settings.*field = std::clamp(settings.*field, 0, StylisticEffects::MaximumIntensity);
    return settings;
}

void applyStylistic(std::vector<LinearColor>& colors, QSize size, quint64 sequence, const StylisticEffects& settings) {
    const int width = size.width(), height = size.height();
    applyVhs(colors, width, height, sequence, amount(settings.vhs));
    applyAberration(colors, width, height, amount(settings.chromaticAberration));
    applyBloom(colors, width, height, amount(settings.bloom));
    if (settings.sepia) applySepia(colors);
    applyGrain(colors, width, height, sequence, amount(settings.grain));
}
}
