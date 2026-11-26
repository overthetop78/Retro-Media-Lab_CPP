#include "FixedPixelProcessing.h"
#include "ColorTransfer.h"

#include <algorithm>
#include <cmath>
#include <tuple>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
float fraction(float value) { return value - std::floor(value); }
}

FixedPixelProfile normalized(FixedPixelProfile profile) {
    constexpr std::array fields{&FixedPixelProfile::gridIntensity, &FixedPixelProfile::pixelGap,
        &FixedPixelProfile::persistenceIntensity, &FixedPixelProfile::backlightBleedIntensity, &FixedPixelProfile::oledEmissionIntensity};
    for (const auto field : fields) profile.*field = std::clamp(profile.*field, 0, FixedPixelProfile::PercentMaximum);
    if (profile.backlightIntensity) profile.backlightIntensity = std::clamp(*profile.backlightIntensity, 0, FixedPixelProfile::PercentMaximum);
    if (profile.blackDepth) profile.blackDepth = std::clamp(*profile.blackDepth, 0, FixedPixelProfile::PercentMaximum);
    profile.responseTimeMilliseconds = std::clamp(profile.responseTimeMilliseconds, 0, FixedPixelProfile::ResponseMaximumMilliseconds);
    return profile;
}

std::vector<quint8> ledBacklightZones(const QImage& source, LedBacklightType type) {
    if (type != LedBacklightType::FullArray && type != LedBacklightType::MiniLed) return {};
    const int width = source.width(), height = source.height();
    const bool mini = type == LedBacklightType::MiniLed;
    constexpr QSize FullArrayGrid{8, 5}, MiniLedGrid{32, 18};
    const auto grid = mini ? MiniLedGrid : FullArrayGrid;
    const int columns = std::min(width, grid.width()), rows = std::min(height, grid.height());
    std::vector<quint8> peaks(size_t(columns) * size_t(rows));
    for (int y = 0; y < height; ++y) {
        const auto start = size_t(y * rows / height) * columns;
        for (int x = 0; x < width; ++x) {
            const auto color = source.pixel(x, y);
            auto& peak = peaks[start + x * columns / width];
            peak = std::max(peak, quint8(std::max({qRed(color), qGreen(color), qBlue(color)})));
        }
    }
    const auto axis = [](int pixel, int length, int zones) {
        const float position = std::clamp((pixel + .5f) * zones / length - .5f, 0.f, zones - 1.f);
        const int first = int(position);
        return std::tuple{first, std::min(first + 1, zones - 1), position - first};
    };
    const float minimum = mini ? .06f : .17f;
    const float diffusion = mini ? .35f : .7f;
    constexpr float SignalMaximum = 255.f;
    std::vector<quint8> levels(size_t(width) * size_t(height));
    for (int y = 0; y < height; ++y) {
        const auto start = size_t(y * rows / height) * columns;
        const auto [top, bottom, verticalBlend] = axis(y, height, rows);
        for (int x = 0; x < width; ++x) {
            const auto [left, right, horizontalBlend] = axis(x, width, columns);
            const auto upper = std::lerp(float(peaks[top * columns + left]), float(peaks[top * columns + right]), horizontalBlend);
            const auto lower = std::lerp(float(peaks[bottom * columns + left]), float(peaks[bottom * columns + right]), horizontalBlend);
            const auto spread = std::lerp(upper, lower, verticalBlend);
            const auto level = std::max(float(peaks[start + x * columns / width]), spread * diffusion) / SignalMaximum;
            levels[size_t(y) * width + x] = quint8(std::nearbyint(SignalMaximum * std::lerp(minimum, 1.f, level)));
        }
    }
    return levels;
}

LcdOptics lcdOptics(LcdPanelType panel) {
    switch (panel) {
    case LcdPanelType::Tn: return {.005f, .0036f, 5.f};
    case LcdPanelType::Va: return {.0004f, .0011f, 11.f};
    default: return {.0018f, .0048f, 8.f};
    }
}

float oledLimiter(OledPanelType panel) {
    return panel == OledPanelType::Woled ? .32f : panel == OledPanelType::QdOled ? .22f : .10f;
}

void applyLedBacklight(std::vector<LinearColor>& colors, QSize output, QSize source, const std::vector<quint8>& zones,
        const LedBacklightProfile& profile) {
    const float gain = std::pow(float(std::clamp(profile.intensity, 0, FixedPixelProfile::PercentMaximum)) /
        FixedPixelProfile::PercentMaximum, .8f);
    const float black = lcdOptics(profile.panelType).black;
    const bool edgeLighting = profile.type == LedBacklightType::Edge;
    constexpr float SignalMaximum = 255.f;
    for (int y = 0; y < output.height(); ++y) {
        const float v = (y + .5f) / output.height();
        const auto sourceRow = size_t(std::min(source.height() - 1, int(v * source.height()))) * source.width();
        const float edgeFalloff = edgeLighting ? std::exp(-9.f * std::min(v, 1.f - v)) : 0.f;
        const float directLight = .98f + .02f * (1.f - std::abs(2.f * v - 1.f));
        for (int x = 0; x < output.width(); ++x) {
            const float u = (x + .5f) / output.width();
            const float edge = edgeFalloff * (.70f + .30f * std::sin(u * 11.f + v * 2.f));
            const float light = edgeLighting ? .92f + .08f * edge : zones.empty() ? directLight
                : zones[sourceRow + std::min(source.width() - 1, int(u * source.width()))] / SignalMaximum;
            auto& color = colors[size_t(y) * output.width() + x];
            const float luma = .2126f * color[0] + .7152f * color[1] + .0722f * color[2];
            const float dark = std::clamp(1.f - luma / .22f, 0.f, 1.f);
            const float floor = black * (1.f + 1.25f * edge * dark * dark);
            for (auto& component : color)
                component = std::clamp(gain * light * (floor + component * (1.f - floor)), 0.f, 1.f);
        }
    }
}

namespace {
float unit(float x) { return std::clamp(x, 0.f, 1.f); }
float smooth(float x) {
    x = unit(x);
    return x * x * (3.f - 2.f * x);
}
float luma(const LinearColor& c) { return .2126f * c[0] + .7152f * c[1] + .0722f * c[2]; }

}

float periodicCoverage(float center, float span, float first, float last) {
    const auto area = [first, last](float position) {
        const float cell = std::floor(position);
        return cell * (last - first) + std::clamp(position - cell - first, 0.f, last - first);
    };
    return unit((area(center + span * .5f) - area(center - span * .5f)) / span);
}

LinearColor cellAxis(int pixel, int cells, int pixels, bool striped, bool reverse, float gap) {
    if (pixels <= cells) return {1.f, 1.f, 1.f};
    constexpr float Inset = .03f, Open = 1.f - 2.f * Inset, Stripe = Open / 3.f;
    const float scale = float(pixels) / cells;
    float span = float(cells) / pixels;
    float center = fraction((pixel + .5f) * span);
    const float aperture = std::lerp(Open, periodicCoverage(center, span, Inset, 1.f - Inset), smooth(scale - 1.f));
    if (!striped || pixels < 2 * qint64(cells)) return {aperture, aperture, aperture};
    const float resolved = .08f + .92f * smooth((scale - 3.f) / 2.f);
    const int cell = int((2 * qint64(pixel) + 1) * cells / (2 * qint64(pixels)));
    const auto boundary = [pixels, cells](int item) {
        return int((2 * qint64(item) * pixels + cells - 1) / (2 * qint64(cells)));
    };
    const int first = boundary(cell);
    span = 1.f / (boundary(cell + 1) - first);
    center = (pixel - first + .5f) * span;
    LinearColor mask;
    for (size_t i = 0; i < mask.size(); ++i) {
        const auto channel = reverse ? mask.size() - 1 - i : i;
        const auto start = Inset + channel * Stripe;
        mask[i] = std::lerp(aperture, periodicCoverage(center, span, start + gap, start + Stripe - gap), resolved);
    }
    return mask;
}

namespace {
float opening(float at, float span, float first, float last) {
    return unit((std::min(at + span * .5f, last) - std::max(at - span * .5f, first)) / span);
}

LinearColor oledEmission(const LinearColor& color, OledPixelStructure structure, float x, float y,
        float spanX, float spanY, bool red) {
    const auto horizontal = [=](float first, float last) { return opening(x, spanX, first, last); };
    const auto vertical = [=](float first, float last) { return opening(y, spanY, first, last); };
    if (structure == OledPixelStructure::Wrgb) {
        const float white = std::min({color[0], color[1], color[2]}) * .60f;
        const float shared = white * horizontal(.030f, .265f);
        const float height = vertical(.030f, .970f);
        return {
            ((color[0] - white) * horizontal(.265f, .500f) + shared) * height,
            ((color[1] - white) * horizontal(.500f, .735f) + shared) * height,
            ((color[2] - white) * horizontal(.735f, .970f) + shared) * height
        };
    }
    if (structure == OledPixelStructure::TriangularRgb) {
        const float lower = vertical(.56f, .92f);
        return {color[0] * lower * horizontal(.03f, .47f),
            color[1] * vertical(.08f, .44f) * horizontal(.28f, .72f),
            color[2] * lower * horizontal(.53f, .97f)};
    }
    const auto diamond = [=](float centerX, float radiusX, float radiusY) {
        return unit(.5f + (1.f - std::abs(x - centerX) / radiusX - std::abs(y - .5f) / radiusY) /
            std::max(spanX / radiusX, spanY / radiusY));
    };
    const float green = diamond(.23f, .20f, .36f);
    const float chroma = diamond(.69f, .30f, .48f);
    return {red ? color[0] * chroma : 0.f, color[1] * green, red ? 0.f : color[2] * chroma};
}
}

float oledFrameLight(const QImage& source) {
    constexpr double MaximumSignal = 255.;
    const auto count = qint64(source.width()) * source.height();
    if (!count) return 0.f;
    quint64 sum = 0;
    for (int y = 0; y < source.height(); ++y) {
        const auto* line = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        for (int x = 0; x < source.width(); ++x) {
            const int light = std::max({qRed(line[x]), qGreen(line[x]), qBlue(line[x])});
            sum += light * light;
        }
    }
    return float(std::nearbyint(sum / (MaximumSignal * count)) / MaximumSignal);
}

void applyOledLight(std::vector<LinearColor>& colors, OledPanelType panel, float frameLight) {
    const float limiter = oledLimiter(panel);
    const float gain = 1.f - limiter * smooth((frameLight - .18f) / .72f);
    for (auto& color : colors) {
        if (panel == OledPanelType::Woled) {
            const float peak = std::max({color[0], color[1], color[2]});
            const float white = std::min({color[0], color[1], color[2]});
            const float desaturation = .25f * smooth((peak - .45f) / .55f) * smooth(white / .50f);
            const float gray = luma(color);
            for (auto& channel : color) channel = std::lerp(channel, gray, desaturation);
        }
        for (auto& channel : color) channel = unit(channel * gain);
    }
}

void applyLcdStructure(std::vector<LinearColor>& colors, QSize output, QSize panel,
        LcdPixelStructure structure, LcdPixelOrientation orientation) {
    if (structure == LcdPixelStructure::None) return;
    const bool mono = structure == LcdPixelStructure::Monochrome;
    if (!mono && output.width() <= panel.width() && output.height() <= panel.height()) return;
    const bool reverse = structure == LcdPixelStructure::Bgr;
    const bool horizontal = orientation == LcdPixelOrientation::Horizontal;
    std::vector<LinearColor> columns(size_t(output.width()));
    for (int x = 0; x < output.width(); ++x)
        columns[size_t(x)] = cellAxis(x, panel.width(), output.width(), !mono && !horizontal, reverse);
    for (int y = 0; y < output.height(); ++y) {
        const auto row = cellAxis(y, panel.height(), output.height(), !mono && horizontal, reverse);
        for (int x = 0; x < output.width(); ++x) {
            auto& color = colors[size_t(y) * output.width() + x];
            if (mono) color.fill(luma(color));
            for (size_t channel = 0; channel < color.size(); ++channel)
                color[channel] *= columns[size_t(x)][channel] * row[channel];
        }
    }
}

void applyLcdLight(std::vector<LinearColor>& colors, QSize size, const LcdProfile& profile) {
    const float gain = std::pow(float(std::clamp(profile.intensity, 0, FixedPixelProfile::PercentMaximum)) /
        FixedPixelProfile::PercentMaximum, .8f);
    const auto optics = lcdOptics(profile.panelType);
    const float spread = std::max(1.f, size.height() * .055f);
    const auto band = [](float distance) { return smooth(1.f - std::abs(distance)); };
    for (int y = 0; y < size.height(); ++y) {
        const float topDistance = (y + .5f) / spread, bottomDistance = (size.height() - y - .5f) / spread;
        const float topTight = std::exp(-3.6f * topDistance), topBroad = std::exp(-2.f * topDistance);
        const float bottomTight = std::exp(-3.8f * bottomDistance), bottomBroad = std::exp(-2.3f * bottomDistance);
        for (int x = 0; x < size.width(); ++x) {
            const float u = (x + .5f) / size.width();
            const float top = .20f + .55f * band((u - .28f) * 2.8f) + .20f * band((u - .87f) * 4.5f);
            const float bottom = .12f + .65f * band((u - .63f) * 2.5f) + .27f * band((u - .10f) * 3.f);
            const float edge = .76f * top * std::lerp(topTight, topBroad, top) +
                bottom * std::lerp(bottomTight, bottomBroad, bottom);
            auto& color = colors[size_t(y) * size.width() + x];
            const float dark = unit(1.f - luma(color) / .22f);
            const float floor = optics.black + optics.leak * edge * dark * dark;
            for (auto& channel : color)
                channel = unit(gain * (floor + channel * (1.f - floor)));
        }
    }
}

void applyOledStructure(std::vector<LinearColor>& colors, QSize output, QSize panel, OledPixelStructure structure) {
    if (structure == OledPixelStructure::None) return;
    if (structure == OledPixelStructure::RgbVertical || structure == OledPixelStructure::BgrVertical ||
        structure == OledPixelStructure::Monochrome) {
        const auto lcd = structure == OledPixelStructure::RgbVertical ? LcdPixelStructure::Rgb :
            structure == OledPixelStructure::BgrVertical ? LcdPixelStructure::Bgr : LcdPixelStructure::Monochrome;
        applyLcdStructure(colors, output, panel, lcd, LcdPixelOrientation::Vertical);
        return;
    }
    const float scaleX = float(output.width()) / panel.width();
    const float scaleY = float(output.height()) / panel.height();
    const float spanX = 1.f / scaleX, spanY = 1.f / scaleY;
    const float detail = structure == OledPixelStructure::Wrgb ?
        smooth((scaleX - 3.5f) / 2.5f) : smooth((std::min(scaleX, scaleY) - 2.f) / 2.f);
    if (scaleX <= 1.f && scaleY <= 1.f) return;
    std::vector<LinearColor> columns(size_t(output.width()));
    for (int x = 0; x < output.width(); ++x)
        columns[size_t(x)] = cellAxis(x, panel.width(), output.width(), false, false);
    for (int y = 0; y < output.height(); ++y) {
        const float pointY = (y + .5f) * spanY;
        const float row = cellAxis(y, panel.height(), output.height(), false, false)[0];
        for (int x = 0; x < output.width(); ++x) {
            auto& color = colors[size_t(y) * output.width() + x];
            const float pointX = (x + .5f) * spanX;
            const auto emission = detail <= 0.f ? LinearColor{} : oledEmission(color, structure, fraction(pointX),
                fraction(pointY), spanX, spanY, (int(pointX) + int(pointY)) % 2 == 0);
            for (size_t channel = 0; channel < color.size(); ++channel)
                color[channel] = std::lerp(color[channel] * columns[size_t(x)][channel] * row, emission[channel], detail);
        }
    }
}

void applyLcdResponse(std::vector<LinearColor>& colors, QSize output, LcdPanelType panel,
        std::chrono::nanoseconds timestamp, FixedPixelHistory& history, bool advance) {
    if (history.size == output && history.colors.size() == colors.size() && timestamp >= history.timestamp) {
        if (!advance && timestamp == history.timestamp) {
            colors = history.colors;
            return;
        }
        const double elapsed = std::max(.001, std::chrono::duration<double, std::milli>(timestamp - history.timestamp).count());
        const float base = lcdOptics(panel).responseMilliseconds;
        const auto blend = [elapsed](float milliseconds) { return float(-std::expm1(-elapsed / milliseconds)); };
        const float response = blend(base);
        for (size_t i = 0; i < colors.size(); ++i) {
            auto& color = colors[i];
            const auto& previous = history.colors[i];
            const float peak = std::min(std::ranges::max(previous), std::ranges::max(color));
            const float weight = panel == LcdPanelType::Va ? blend(base * (1.f + 2.2f * (1.f - unit(peak / .2f)))) : response;
            for (size_t channel = 0; channel < color.size(); ++channel)
                color[channel] = std::lerp(previous[channel], color[channel], weight);
        }
    }
    if (advance) history = {output, timestamp, colors};
}
}
