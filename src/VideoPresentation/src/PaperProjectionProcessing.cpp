#include "PaperProjectionProcessing.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr int Channels = 3;
float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float luminance(const LinearColor& color) { return color[0] * .2126f + color[1] * .7152f + color[2] * .0722f; }
float quantize(float value, float levels) { return std::floor(value * levels + .5f) / levels; }

std::size_t index(QSize size, int x, int y) {
    return std::size_t(y) * std::size_t(size.width()) + std::size_t(x);
}

}

std::array<float, 4> ePaperSurface(EPaperColorMode mode) {
    switch (mode) {
    case EPaperColorMode::Monochrome: return {.83f, .82f, .76f, .025f};
    case EPaperColorMode::Grayscale16: return {.82f, .81f, .76f, .035f};
    case EPaperColorMode::TricolorRed:
    case EPaperColorMode::TricolorYellow: return {.84f, .83f, .77f, .030f};
    case EPaperColorMode::Color4096: return {.67f, .67f, .63f, .072f};
    default: return {.83f, .82f, .77f, .032f};
    }
}

EPaperRefresh EPaperRefreshClock::advance(const EPaperProfile& profile, QSize size, std::chrono::nanoseconds timestamp) {
    if (profile_ != profile || size_ != size || timestamp < timestamp_) {
        profile_ = profile;
        size_ = size;
        timestamp_ = refreshStarted_ = timestamp;
        refreshCount_ = 0;
        return {true, false, 1.f, 0.f};
    }

    struct Timing { float interval, ghost; int fullEvery; };
    constexpr std::array timings{
        Timing{270.f, .28f, 6}, Timing{390.f, .34f, 6}, Timing{450.f, .31f, 6},
        Timing{720.f, .24f, 4}, Timing{720.f, .24f, 4}, Timing{960.f, .24f, 4}
    };
    const auto [interval, ghost, fullEvery] = timings.at(size_t(profile.colorMode));
    timestamp_ = timestamp;
    float elapsed = std::chrono::duration<float, std::milli>(timestamp - refreshStarted_).count();
    const bool captureHistory = elapsed >= interval;
    if (captureHistory) {
        refreshStarted_ = timestamp;
        refreshCount_ = refreshCount_ % fullEvery + 1;
        elapsed = 0.f;
    }

    constexpr float inversionHalfDuration = 20.f;
    constexpr float inversionDuration = inversionHalfDuration * 2.f;
    const bool fullRefresh = refreshCount_ == fullEvery;
    const float inversion = fullRefresh
        ? unit(1.f - std::abs(elapsed - inversionHalfDuration) / inversionHalfDuration) * .35f : 0.f;
    const float fade = unit((elapsed / interval - .70f) / .30f);
    float residual = ghost * (1.f - fade * fade * (3.f - 2.f * fade));
    if (fullRefresh) residual *= unit((elapsed - inversionDuration) / inversionDuration);
    return {captureHistory, elapsed > 0.f, 1.f - residual, inversion};
}

ProjectionProfile normalized(ProjectionProfile profile) {
    profile.ambientLight = std::clamp(profile.ambientLight, 0, ProjectionProfile::MaximumAmbientLight);
    return profile;
}

void applyEPaper(std::vector<LinearColor>& colors, EPaperColorMode mode) {
    const auto surface = ePaperSurface(mode);
    for (auto& color : colors) {
        for (auto& channel : color) channel = unit(channel);
        const float y = luminance(color);
        const bool color4096 = mode == EPaperColorMode::Color4096;
        if (color4096 || mode == EPaperColorMode::Gallery) {
            const float levels = color4096 ? 15.f : 31.f;
            for (auto& channel : color) channel = quantize(channel, levels);
            const float gray = luminance(color);
            for (auto& channel : color) channel = std::lerp(gray, channel, color4096 ? .56f : .94f);
        } else if (mode == EPaperColorMode::TricolorRed && color[0] > std::max(color[1], color[2]) + .18f && color[0] > .36f) {
            color = {.60f, .031f, .026f};
            continue;
        } else if (mode == EPaperColorMode::TricolorYellow && std::min(color[0], color[1]) > color[2] + .17f &&
                   std::min(color[0], color[1]) > .36f) {
            color = {.81f, .65f, .041f};
            continue;
        } else {
            color.fill(mode == EPaperColorMode::Grayscale16 ? quantize(y, 15.f) : y >= .48f ? 1.f : 0.f);
        }
        for (int channel = 0; channel < Channels; ++channel)
            color[channel] = std::lerp(surface[3], surface[channel], color[channel]);
    }
}

void applyEPaperTemporal(std::vector<LinearColor>& colors, QSize size, std::chrono::nanoseconds timestamp,
        const EPaperProfile& profile, EPaperHistory& history) {
    const auto refresh = history.clock.advance(profile, size, timestamp);
    if (refresh.captureHistory) history.colors = colors;
    if (!refresh.hasPrevious || history.colors.size() != colors.size()) return;

    const auto surface = ePaperSurface(profile.colorMode);
    for (size_t pixel = 0; pixel < colors.size(); ++pixel) {
        auto& color = colors[pixel];
        const auto& previous = history.colors[pixel];
        float change = 0.f;
        for (int channel = 0; channel < Channels; ++channel) {
            const float delta = color[channel] - previous[channel];
            change += delta * delta;
        }
        const float inversion = change > .0001f ? refresh.inversion : 0.f;
        for (int channel = 0; channel < Channels; ++channel) {
            const float pigment = std::lerp(previous[channel], color[channel], refresh.currentWeight);
            color[channel] = std::lerp(pigment, unit(surface[channel] + surface[3] - pigment), inversion);
        }
    }
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

ProjectionOptics projectionOptics(const ProjectionProfile& profile) {
    struct ChannelOptics { float black, blur, border, convergence; };
    constexpr std::array opticsByTechnology{
        ChannelOptics{.0007f, .48f, 0.f, .20f},
        ChannelOptics{.009f, 0.f, .065f, 0.f},
        ChannelOptics{.004f, 0.f, .020f, 0.f},
        ChannelOptics{.003f, 0.f, .020f, 0.f},
        ChannelOptics{.0013f, 0.f, .008f, 0.f}
    };
    const auto optics = opticsByTechnology[size_t(profile.technology)];
    const float reflection = profile.surface == ProjectionSurface::GrayScreen ? .62f : 1.f;
    const float ambient = .25f * profile.ambientLight / ProjectionProfile::MaximumAmbientLight;
    const float texture = profile.surface == ProjectionSurface::WhiteWall ? .045f : 0.f;
    return {optics.black, optics.blur, optics.border, optics.convergence, reflection, ambient, texture};
}

void applyProjection(std::vector<LinearColor>& colors, QSize output, QSize source, QSize panel, const ProjectionProfile& profile) {
    const auto optics = projectionOptics(profile);
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
            const float reflection = optics.texture == 0.f ? optics.reflection : optics.reflection *
                (1.f - optics.texture * (.5f + .25f * std::sin(px * .71f + py * .31f) + .25f * std::sin(px * .17f - py * .83f)));
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
                color[channel] = std::lerp(projected, 1.f, optics.ambient) * reflection;
            }
        }
    }
}
}
