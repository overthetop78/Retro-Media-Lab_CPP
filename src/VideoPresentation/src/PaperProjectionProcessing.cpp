#include "PaperProjectionProcessing.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr int Channels = 3;
constexpr int BayerWidth = 4;
constexpr int GrayLevels = 15;
constexpr int FullIntensity = 100;
constexpr int Bayer[BayerWidth][BayerWidth]{
    {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}
};
constexpr LinearColor Ink{.015f, .018f, .015f};

float intensity(int value) { return std::clamp(value, 0, FullIntensity) / float(FullIntensity); }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float luminance(const LinearColor& color) { return color[0] * .2126f + color[1] * .7152f + color[2] * .0722f; }
float quantize(float value) {
    const float scaled = unit(value) * GrayLevels;
    const int low = int(scaled);
    const float fraction = scaled - float(low);
    // MathF.Round uses ties to even.
    return float(low + (fraction > .5f || (fraction == .5f && (low & 1)))) / GrayLevels;
}

std::size_t index(QSize size, int x, int y) {
    return std::size_t(y) * std::size_t(size.width()) + std::size_t(x);
}

const LinearColor& sample(const std::vector<LinearColor>& source, QSize size, int x, int y) {
    return source[index(size, std::clamp(x, 0, size.width() - 1), std::clamp(y, 0, size.height() - 1))];
}

float paperTexture(int x, int y) {
    constexpr std::uint32_t HashX = 374761393u, HashY = 668265263u, HashMix = 1274126177u;
    auto hash = std::uint32_t(x) * HashX + std::uint32_t(y) * HashY;
    hash = (hash ^ (hash >> 13)) * HashMix;
    return float(hash ^ (hash >> 16)) / float(std::numeric_limits<std::uint32_t>::max());
}
}

EPaperProfile normalized(EPaperProfile profile) {
    for (auto* value : {&profile.contrast, &profile.dithering, &profile.ghosting, &profile.inkDensity,
             &profile.paperBrightness, &profile.paperWarmth, &profile.colorSaturation, &profile.surfaceTexture,
             &profile.edgeSoftness})
        *value = std::clamp(*value, 0, EPaperProfile::MaximumIntensity);
    profile.refreshTimeMilliseconds = std::clamp(profile.refreshTimeMilliseconds, 0, EPaperProfile::MaximumDurationMilliseconds);
    return profile;
}

ProjectionProfile normalized(ProjectionProfile profile) {
    profile.ambientLight = std::clamp(profile.ambientLight, 0, ProjectionProfile::MaximumAmbientLight);
    return profile;
}

void applyEPaper(std::vector<LinearColor>& colors, QSize size, const EPaperProfile& profile) {
    const float softness = intensity(profile.edgeSoftness) * .65f;
    const auto source = softness > 0.f ? colors : std::vector<LinearColor>{};
    const float contrast = .6f + intensity(profile.contrast) * 1.8f;
    const float brightness = .55f + intensity(profile.paperBrightness) * .4f;
    const float warmth = intensity(profile.paperWarmth);
    const LinearColor paper{brightness, brightness * (1.f - warmth * .05f), brightness * (1.f - warmth * .16f)};
    const float density = intensity(profile.inkDensity);
    const float saturation = intensity(profile.colorSaturation);
    const float texture = intensity(profile.surfaceTexture) * .055f;
    const bool monochrome = profile.colorMode == EPaperColorMode::Monochrome;
    const float ditherStrength = intensity(profile.dithering) / (monochrome ? 1 : GrayLevels);

    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
            auto& destination = colors[index(size, x, y)];
            auto levels = destination;
            if (softness > 0.f) {
                const auto& left = sample(source, size, x - 1, y);
                const auto& right = sample(source, size, x + 1, y);
                const auto& above = sample(source, size, x, y - 1);
                const auto& below = sample(source, size, x, y + 1);
                for (int channel = 0; channel < Channels; ++channel)
                    levels[channel] = std::lerp(levels[channel],
                        (left[channel] + right[channel] + above[channel] + below[channel]) * .25f, softness);
            }
            for (auto& level : levels) level = unit((level - .5f) * contrast + .5f);
            const float dither = ((float(Bayer[y % BayerWidth][x % BayerWidth]) + .5f) / (BayerWidth * BayerWidth)
                - .5f) * ditherStrength;
            if (profile.colorMode == EPaperColorMode::Color4096) {
                for (auto& level : levels) level = quantize(level + dither);
                const float gray = luminance(levels);
                for (auto& level : levels) level = std::lerp(gray, level, saturation);
            } else {
                const float gray = luminance(levels) + dither;
                levels.fill(monochrome ? (gray >= .5f ? 1.f : 0.f) : quantize(gray));
            }
            const float grain = (paperTexture(x, y) - .5f) * texture;
            for (int channel = 0; channel < Channels; ++channel) {
                const float dark = std::lerp(paper[channel] * .35f, Ink[channel], density);
                destination[channel] = unit(std::lerp(dark, paper[channel], levels[channel]) + grain);
            }
        }
    }
}

void applyEPaperTemporal(std::vector<LinearColor>& colors, QSize size, std::chrono::nanoseconds timestamp,
    const EPaperProfile& profile, EPaperHistory& history, bool advance) {
    if (history.size == size && history.colors.size() == colors.size() && timestamp >= history.timestamp) {
        if (!advance && timestamp == history.timestamp) {
            colors = history.colors;
            return;
        }
        const auto elapsed = std::chrono::duration<double, std::milli>(timestamp - history.timestamp).count();
        const float response = profile.refreshTimeMilliseconds == 0 ? 1.f :
            float(-std::expm1(-std::max(.001, elapsed) / profile.refreshTimeMilliseconds));
        const float factor = response * (1.f - intensity(profile.ghosting) * .4f);
        for (std::size_t pixel = 0; pixel < colors.size(); ++pixel)
            for (int channel = 0; channel < Channels; ++channel)
                colors[pixel][channel] = std::lerp(history.colors[pixel][channel], colors[pixel][channel], factor);
    }
    if (advance) history = {size, timestamp, colors};
}

QImage separateProjectionColors(const QImage& source, const Emulation::VideoFrame& frame,
        const ProjectionProfile& configuration, const DisplaySignalProfile& signal, ProjectionColorHistory& history) {
    const bool compatible = !history.previous.isNull() && history.previous.size() == source.size() &&
        history.configuration == configuration && history.signal == signal;
    if (compatible && frame.sequence == history.sequence && frame.timestamp == history.timestamp)
        return history.separated.isNull() ? source : history.separated;

    constexpr quint64 MaximumFrameGap = 2;
    constexpr auto MaximumTimeGap = std::chrono::milliseconds(85);
    constexpr float MotionThreshold = 24.f, EdgeRange = 110.f, MaximumStrength = .32f;
    QImage separated;
    if (compatible && frame.sequence > history.sequence && frame.sequence - history.sequence <= MaximumFrameGap &&
        frame.timestamp >= history.timestamp && frame.timestamp - history.timestamp <= MaximumTimeGap) {
        const auto luma = [](QRgb color) {
            return (54 * qRed(color) + 183 * qGreen(color) + 19 * qBlue(color)) >> 8;
        };
        const auto blend = [](int current, int shifted, float strength) {
            return int(std::nearbyint(std::lerp(float(current), float(shifted), strength)));
        };
        for (int y = 0; y < source.height(); ++y) {
            const auto* row = reinterpret_cast<const QRgb*>(source.constScanLine(y));
            const auto* previous = reinterpret_cast<const QRgb*>(history.previous.constScanLine(y));
            const auto* top = reinterpret_cast<const QRgb*>(source.constScanLine(std::max(0, y - 1)));
            const auto* bottom = reinterpret_cast<const QRgb*>(source.constScanLine(std::min(source.height() - 1, y + 1)));
            for (int x = 0; x < source.width(); ++x) {
                const float change = float(luma(row[x]) - luma(previous[x]));
                const float motion = unit((std::abs(change) - MotionThreshold) / EdgeRange);
                if (motion <= 0.f) continue;
                const auto left = row[std::max(0, x - 1)];
                const auto right = row[std::min(source.width() - 1, x + 1)];
                const float horizontalEdge = float(luma(right) - luma(left));
                const float verticalEdge = float(luma(bottom[x]) - luma(top[x]));
                const bool horizontal = std::abs(horizontalEdge) >= std::abs(verticalEdge);
                const float edge = horizontal ? horizontalEdge : verticalEdge;
                const float strength = MaximumStrength * motion * unit((std::abs(edge) - MotionThreshold) / EdgeRange);
                if (strength <= 0.f) continue;
                if (separated.isNull()) {
                    separated = source.copy();
                    if (separated.isNull()) break;
                }
                auto rear = horizontal ? left : top[x];
                auto front = horizontal ? right : bottom[x];
                if (change * edge > 0.f) std::swap(rear, front);
                auto* output = reinterpret_cast<QRgb*>(separated.scanLine(y));
                output[x] = qRgb(blend(qRed(row[x]), qRed(rear), strength), qGreen(row[x]),
                    blend(qBlue(row[x]), qBlue(front), strength));
            }
        }
    }
    history = {configuration, signal, frame.sequence, frame.timestamp, source, std::move(separated)};
    return history.separated.isNull() ? source : history.separated;
}

void applyProjection(std::vector<LinearColor>& colors, QSize output, QSize source, QSize panel, const ProjectionProfile& profile) {
    struct Optics { float black, blur, border, convergence; };
    constexpr std::array opticsByTechnology{
        Optics{.0007f, .48f, 0.f, .20f},
        Optics{.009f, 0.f, .065f, 0.f},
        Optics{.004f, 0.f, .020f, 0.f},
        Optics{.003f, 0.f, .020f, 0.f},
        Optics{.0013f, 0.f, .008f, 0.f}
    };
    const auto optics = opticsByTechnology[size_t(profile.technology)];
    const float reflectedLight = profile.surface == ProjectionSurface::GrayScreen ? .62f : 1.f;
    const float ambient = .25f * profile.ambientLight / ProjectionProfile::MaximumAmbientLight;
    const float texture = profile.surface == ProjectionSurface::WhiteWall ? .045f : 0.f;
    const float stepX = std::max(1.f, .55f * output.width() / panel.width());
    const float stepY = std::max(1.f, .55f * output.height() / panel.height());
    const float convergence = optics.convergence * output.width() / source.width();
    const auto original = optics.blur > 0.f ? colors : std::vector<LinearColor>{};
    const auto aperture = [border = optics.border](int position, int cells, int pixels) {
        if (border <= 0.f || pixels <= cells) return 1.f;
        const float span = float(cells) / pixels;
        const float location = (position + .5f) * span;
        const float center = location - std::floor(location);
        const auto openArea = [border](float value) {
            const float cell = std::floor(value);
            const float opening = 1.f - 2.f * border;
            return cell * opening + std::clamp(value - cell - border, 0.f, opening);
        };
        const float coverage = unit((openArea(center + span * .5f) - openArea(center - span * .5f)) / span);
        const float transition = unit((float(pixels) / cells - 1.f) / 1.5f);
        return std::lerp(1.f, coverage, transition * transition * (3.f - 2.f * transition));
    };
    std::vector<float> columns(size_t(output.width()));
    for (int x = 0; x < output.width(); ++x) columns[size_t(x)] = aperture(x, panel.width(), output.width());
    const auto sampleBeam = [&](float x, float y, int channel) {
        x = std::clamp(x, 0.f, float(output.width() - 1));
        y = std::clamp(y, 0.f, float(output.height() - 1));
        const int left = int(x), top = int(y);
        const int right = std::min(output.width() - 1, left + 1);
        const int bottom = std::min(output.height() - 1, top + 1);
        const auto upper = std::lerp(original[index(output, left, top)][channel],
            original[index(output, right, top)][channel], x - left);
        const auto lower = std::lerp(original[index(output, left, bottom)][channel],
            original[index(output, right, bottom)][channel], x - left);
        return std::lerp(upper, lower, y - top);
    };
    for (int y = 0; y < output.height(); ++y) {
        const float row = aperture(y, panel.height(), output.height());
        for (int x = 0; x < output.width(); ++x) {
            auto& color = colors[index(output, x, y)];
            const float area = columns[size_t(x)] * row;
            const float px = x + .5f, py = y + .5f;
            const float reflection = texture == 0.f ? reflectedLight : reflectedLight *
                (1.f - texture * (.5f + .25f * std::sin(px * .71f + py * .31f) + .25f * std::sin(px * .17f - py * .83f)));
            for (int channel = 0; channel < Channels; ++channel) {
                float value = color[channel];
                if (optics.blur > 0.f) {
                    const float beamX = x + (channel - 1) * convergence;
                    const float center = sampleBeam(beamX, float(y), channel);
                    const float surround = .25f * (sampleBeam(beamX - stepX, float(y), channel) +
                        sampleBeam(beamX + stepX, float(y), channel) +
                        sampleBeam(beamX, y - stepY, channel) + sampleBeam(beamX, y + stepY, channel));
                    value = std::lerp(center, surround, optics.blur);
                }
                const float projected = std::lerp(optics.black, 1.f, value * area);
                color[channel] = std::lerp(projected, 1.f, ambient) * reflection;
            }
        }
    }
}
}
