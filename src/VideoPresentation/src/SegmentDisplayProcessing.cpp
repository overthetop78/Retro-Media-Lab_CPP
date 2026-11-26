#include "SegmentDisplayProcessing.h"

#include <QVector2D>
#include <algorithm>
#include <cmath>
#include <limits>

namespace RetroMediaLab::VideoPresentation {

std::span<const SegmentColor> segmentColors(SegmentDisplayTechnology technology) {
    static constexpr std::array lcd{SegmentColor::Black, SegmentColor::Gray};
    static constexpr std::array led{SegmentColor::Red, SegmentColor::Green, SegmentColor::Amber, SegmentColor::Blue, SegmentColor::White};
    static constexpr std::array vfd{SegmentColor::BlueGreen, SegmentColor::Green, SegmentColor::Blue, SegmentColor::White, SegmentColor::Amber};
    switch (technology) {
    case SegmentDisplayTechnology::Lcd: return lcd;
    case SegmentDisplayTechnology::Vfd: return vfd;
    default: return led;
    }
}

}

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
struct Element { QVector2D start, end; bool point = false; };

constexpr Element Top{{.2f,.08f},{.8f,.08f}}, Bottom{{.2f,.92f},{.8f,.92f}};
constexpr Element UpperRight{{.82f,.1f},{.82f,.48f}}, LowerRight{{.82f,.52f},{.82f,.9f}};
constexpr Element LowerLeft{{.18f,.52f},{.18f,.9f}}, UpperLeft{{.18f,.1f},{.18f,.48f}};
constexpr Element Middle{{.2f,.5f},{.8f,.5f}};
constexpr std::array Seven{Top, UpperRight, LowerRight, Bottom, LowerLeft, UpperLeft, Middle};
constexpr std::array Inner{
    Element{{.2f,.5f},{.48f,.5f}}, Element{{.52f,.5f},{.8f,.5f}},
    Element{{.2f,.1f},{.46f,.46f}}, Element{{.8f,.1f},{.54f,.46f}},
    Element{{.2f,.9f},{.46f,.54f}}, Element{{.8f,.9f},{.54f,.54f}},
    Element{{.5f,.1f},{.5f,.46f}}, Element{{.5f,.54f},{.5f,.9f}}
};
constexpr std::array SplitOuter{
    Element{{.2f,.08f},{.48f,.08f}}, Element{{.52f,.08f},{.8f,.08f}}, UpperRight, LowerRight,
    Element{{.2f,.92f},{.48f,.92f}}, Element{{.52f,.92f},{.8f,.92f}}, LowerLeft, UpperLeft
};
constexpr std::array<LinearColor, 8> Tints{{
    {1.f,.025f,.01f}, {.04f,1.f,.08f}, {1.f,.42f,.015f}, {.03f,.28f,1.f}, {1.f,1.f,1.f},
    {.012f,.015f,.010f}, {.16f,.17f,.155f}, {.085f,1.f,.73f}
}};
constexpr int ActivationSamples = 5;

struct SegmentOptics {
    SegmentDisplayTechnology technology;
    LinearColor tint;
    float background;

    LinearColor compose(float level, float core, float halo) const {
        constexpr LinearColor LcdPanel{.57f, .61f, .48f}, VfdPanel{.002f, .0045f, .0055f};
        constexpr float InactiveLcd = .92f, UnlitSegment = .006f, Emission = .94f;
        LinearColor color;
        for (size_t c = 0; c < color.size(); ++c)
            color[c] = technology == SegmentDisplayTechnology::Lcd
                ? std::lerp(LcdPanel[c], std::lerp(LcdPanel[c] * InactiveLcd, tint[c], level), core)
                : std::min(1.f, (technology == SegmentDisplayTechnology::Vfd ? VfdPanel[c] : background) +
                    tint[c] * (core * (UnlitSegment + Emission * level) + halo));
        return color;
    }
};

float amount(int setting) { return float(setting) / SegmentDisplayProfile::MaximumIntensity; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }

QSize segmentCellSize(const SegmentDisplayProfile& profile) {
    const int width = 6 + int(std::nearbyint(amount(profile.cellSize) * 26.f));
    return {width, std::max(9, int(std::nearbyint(width * 1.55f)))};
}

SegmentEndShape segmentEndShape(const SegmentDisplayProfile& profile) {
    if (profile.technology == SegmentDisplayTechnology::Lcd) return SegmentEndShape::Straight;
    if (profile.technology == SegmentDisplayTechnology::Vfd) return SegmentEndShape::Rounded;
    return profile.endShape;
}

std::vector<Element> makeElements(const SegmentDisplayProfile& profile) {
    std::vector<Element> result;
    if (profile.layout == SegmentLayout::Sixteen) result.assign(SplitOuter.begin(), SplitOuter.end());
    else result.assign(Seven.begin(), profile.layout == SegmentLayout::Seven ? Seven.end() : Seven.end() - 1);
    if (profile.layout != SegmentLayout::Seven) result.insert(result.end(), Inner.begin(), Inner.end());
    constexpr int LcdGap = 8, VfdGap = 4;
    int gapSetting = profile.segmentGap;
    if (profile.technology == SegmentDisplayTechnology::Lcd)
        gapSetting = std::min(SegmentDisplayProfile::MaximumIntensity, gapSetting + LcdGap);
    if (profile.technology == SegmentDisplayTechnology::Vfd) gapSetting = std::max(0, gapSetting - VfdGap);
    const float gap = amount(gapSetting) * .12f;
    for (auto& line : result) {
        const auto shortening = (line.end - line.start) * gap;
        line.start += shortening;
        line.end -= shortening;
    }
    if (profile.decimalPoint) result.push_back({{.91f,.9f},{.91f,.9f},true});
    if (profile.colon) {
        result.push_back({{.91f,.35f},{.91f,.35f},true});
        result.push_back({{.91f,.65f},{.91f,.65f},true});
    }
    return result;
}

float distance(QVector2D p, const Element& line, SegmentEndShape shape) {
    p -= line.start;
    if (line.point) return p.length();
    const auto direction = line.end - line.start;
    const float position = QVector2D::dotProduct(p, direction) / direction.lengthSquared();
    if (shape == SegmentEndShape::Rounded) return (p - direction * unit(position)).length();
    const float side = (p - direction * position).length();
    const float extension = std::max(0.f, std::abs(position - .5f) - .5f) * direction.length();
    return std::max(side, shape == SegmentEndShape::Straight ? extension : (extension + side) * .70710678f);
}
}

LinearColor segmentTint(SegmentColor color) { return Tints[size_t(color)]; }

SegmentDisplayProfile normalized(SegmentDisplayProfile profile) {
    const auto colors = segmentColors(profile.technology);
    if (std::ranges::find(colors, profile.color) == colors.end()) profile.color = colors.front();
    for (const auto field : {&SegmentDisplayProfile::thickness, &SegmentDisplayProfile::contrast, &SegmentDisplayProfile::glow,
            &SegmentDisplayProfile::cellSize, &SegmentDisplayProfile::horizontalGap, &SegmentDisplayProfile::verticalGap,
            &SegmentDisplayProfile::segmentGap, &SegmentDisplayProfile::brightness, &SegmentDisplayProfile::activationThreshold,
            &SegmentDisplayProfile::offSegmentVisibility, &SegmentDisplayProfile::blackDepth, &SegmentDisplayProfile::haloRadius})
        profile.*field = std::clamp(profile.*field, 0, SegmentDisplayProfile::MaximumIntensity);
    for (const auto field : {&SegmentDisplayProfile::responseTimeMilliseconds, &SegmentDisplayProfile::persistenceMilliseconds})
        profile.*field = std::clamp(profile.*field, 0, SegmentDisplayProfile::MaximumDurationMilliseconds);
    return profile;
}

RasterSegmentParameters rasterSegmentParameters(const SegmentDisplayProfile& profile, QSize source, double elapsedMilliseconds) {
    RasterSegmentParameters parameters;
    const auto cell = segmentCellSize(profile);
    const auto elements = makeElements(profile);
    const bool vfd = profile.technology == SegmentDisplayTechnology::Vfd;
    parameters.geometry = {float(cell.width()), float(cell.height()),
        1.f - amount(profile.horizontalGap) * .42f, 1.f - amount(profile.verticalGap) * .42f};
    parameters.shape = {.018f + amount(profile.thickness) * .105f, float(segmentEndShape(profile)),
        float(elements.size()), (1.f - amount(profile.blackDepth)) * .08f};
    parameters.light = {amount(profile.activationThreshold), .5f + amount(profile.contrast) * 3.5f,
        4.f / (.025f + amount(profile.haloRadius) * (vfd ? 1.f : .34f) * .24f), vfd ? .32f : .09f};
    const double elapsed = std::max(.001, elapsedMilliseconds);
    const float response = profile.responseTimeMilliseconds > 0
        ? float(-std::expm1(-elapsed / profile.responseTimeMilliseconds)) : 1.f;
    const float decay = profile.persistenceMilliseconds > 0
        ? float(std::exp(-elapsed / profile.persistenceMilliseconds)) : 0.f;
    parameters.timing = {float(source.width()), float(source.height()), response, decay};
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& element = elements[i];
        parameters.lines[i] = {element.start.x(), element.start.y(), element.end.x(), element.end.y()};
    }
    return parameters;
}

void applySegmentDisplay(std::vector<LinearColor>& colors, QSize output, QSize source, const SegmentDisplayProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const auto cellSize = segmentCellSize(profile);
    const int w = cellSize.width(), h = cellSize.height();
    const int columns = (source.width() - 1) / w + 1, rows = (source.height() - 1) / h + 1;
    const auto elements = makeElements(profile);
    const auto cellOffset = [&](int x, int y) { return (size_t(y) * columns + x) * elements.size(); };
    std::vector<float> activations(size_t(columns) * rows * elements.size());
    const float threshold = amount(profile.activationThreshold);
    const float contrast = .5f + amount(profile.contrast) * 3.5f;
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < columns; ++x)
            for (size_t i = 0; i < elements.size(); ++i) {
                const auto& element = elements[i];
                const int count = element.point ? 1 : ActivationSamples;
                float sum = 0;
                for (int sample = 0; sample < count; ++sample) {
                    const float t = element.point ? 0.f : (sample + .5f) / count;
                    const auto p = element.start + (element.end - element.start) * t;
                    const float sx = std::clamp((x + p.x()) * w, 0.f, float(source.width() - 1));
                    const float sy = std::clamp((y + p.y()) * h, 0.f, float(source.height() - 1));
                    const int ox = std::min(output.width() - 1, int(sx * output.width() / source.width()));
                    const int oy = std::min(output.height() - 1, int(sy * output.height() / source.height()));
                    const auto& c = colors[size_t(oy) * output.width() + ox];
                    sum += c[0] * .2126f + c[1] * .7152f + c[2] * .0722f;
                }
                activations[cellOffset(x,y) + i] = unit((sum / count - threshold) * contrast + .5f);
            }
    const float rx = float(source.width()) / output.width(), ry = float(source.height()) / output.height();
    const float edge = std::max(rx, ry) / std::min(w, h);
    const float radius = .018f + amount(profile.thickness) * .105f;
    const float gapX = 1.f - amount(profile.horizontalGap) * .42f, gapY = 1.f - amount(profile.verticalGap) * .42f;
    const bool vfd = profile.technology == SegmentDisplayTechnology::Vfd;
    const auto endShape = segmentEndShape(profile);
    constexpr int NonVfdHaloDivisor = 3;
    constexpr float VfdHaloGain = .32f, LedHaloGain = .09f;
    const float halo = 4.f / (.025f + amount(vfd ? profile.haloRadius : profile.haloRadius / NonVfdHaloDivisor) * .24f);
    const float background = (1.f - amount(profile.blackDepth)) * .08f;
    const SegmentOptics optics{profile.technology, segmentTint(profile.color), background};
    for (int y = 0; y < output.height(); ++y) {
        const float posY = (y + .5f) * ry / h;
        const int cellY = int(posY);
        const float localY = (posY - cellY - .5f) / gapY + .5f;
        for (int x = 0; x < output.width(); ++x) {
            const float posX = (x + .5f) * rx / w;
            const int cellX = int(posX);
            const QVector2D local((posX - cellX - .5f) / gapX + .5f, localY);
            float nearest = std::numeric_limits<float>::max();
            size_t active = 0;
            for (size_t i = 0; i < elements.size(); ++i) {
                const float d = distance(local, elements[i], endShape);
                if (d < nearest) { nearest = d; active = i; }
            }
            const float t = unit((radius + edge - nearest) / (2.f * edge));
            const float core = t * t * (3.f - 2.f * t);
            const auto cell = cellOffset(std::min(columns - 1, cellX), std::min(rows - 1, cellY));
            const float level = activations[cell + active];
            const float spill = std::exp(-std::max(0.f, nearest - radius) * halo) *
                (vfd ? VfdHaloGain : LedHaloGain) * level * level * (1.f - core);
            colors[size_t(y) * output.width() + x] = optics.compose(level, core, spill);
        }
    }
}

bool applyNativeSegments(std::vector<LinearColor>& colors, QSize output, const SegmentDisplayProfile& profile,
        const Emulation::VideoFrame& frame, Emulation::VideoSegmentLevels levels) {
    constexpr int PixelBytes = sizeof(quint32);
    constexpr int RedOffset = 2;
    if (frame.pixelFormat != Emulation::PixelFormat::Xrgb8888 || levels.onRed <= levels.offRed ||
        frame.width <= 0 || frame.height <= 0 || qint64(frame.width) * PixelBytes > frame.pitch ||
        qint64(frame.height - 1) * frame.pitch + qint64(frame.width) * PixelBytes > frame.pixels.size() ||
        output.isEmpty() || colors.size() != size_t(output.width()) * output.height()) return false;

    constexpr float NativeBackground = .004f;
    const SegmentOptics optics{profile.technology, segmentTint(profile.color), NativeBackground};
    const float range = levels.onRed - levels.offRed;
    for (int y = 0; y < output.height(); ++y) {
        const auto sy = int((2 * qint64(y) + 1) * frame.height / (2 * qint64(output.height())));
        const auto* row = reinterpret_cast<const quint8*>(frame.pixels.constData()) + qsizetype(sy) * frame.pitch;
        for (int x = 0; x < output.width(); ++x) {
            const auto sx = int((2 * qint64(x) + 1) * frame.width / (2 * qint64(output.width())));
            const auto red = row[qsizetype(sx) * PixelBytes + RedOffset];
            const float level = unit((red - levels.offRed) / range);
            colors[size_t(y) * output.width() + x] = optics.compose(level, red ? 1.f : 0.f, 0.f);
        }
    }
    return true;
}

void applySegmentTemporal(std::vector<LinearColor>& colors, QSize size, std::chrono::nanoseconds timestamp,
        const SegmentDisplayProfile& profile, SegmentHistory& history, bool advance) {
    if (!profile.responseTimeMilliseconds && !profile.persistenceMilliseconds) return;
    if (history.size == size && history.colors.size() == colors.size() && timestamp >= history.timestamp) {
        if (!advance && timestamp == history.timestamp) { colors = history.colors; return; }
        const auto elapsed = std::max(.001, std::chrono::duration<double, std::milli>(timestamp - history.timestamp).count());
        const float response = profile.responseTimeMilliseconds > 0
            ? float(-std::expm1(-elapsed / profile.responseTimeMilliseconds)) : 1.f;
        const float decay = profile.persistenceMilliseconds > 0 ? float(std::exp(-elapsed / profile.persistenceMilliseconds)) : 0.f;
        for (size_t i = 0; i < colors.size(); ++i)
            for (size_t c = 0; c < colors[i].size(); ++c) {
                const float previous = history.colors[i][c];
                auto& value = colors[i][c];
                value = value >= previous ? std::lerp(previous, value, response) : std::max(value, previous * decay);
            }
    }
    if (advance) history = {size, timestamp, colors};
}
}
