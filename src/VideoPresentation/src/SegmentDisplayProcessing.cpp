#include "SegmentDisplayProcessing.h"

#include <QVector2D>
#include <algorithm>
#include <cmath>
#include <limits>

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
constexpr std::array<LinearColor, 5> Tints{{
    {1.f,.025f,.01f}, {.04f,1.f,.08f}, {1.f,.42f,.015f}, {.03f,.28f,1.f}, {1.f,1.f,1.f}
}};
constexpr int ActivationSamples = 5;

float amount(int setting) { return float(setting) / SegmentDisplayProfile::MaximumIntensity; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }

std::vector<Element> makeElements(const SegmentDisplayProfile& profile) {
    std::vector<Element> result;
    if (profile.layout == SegmentLayout::Sixteen) result.assign(SplitOuter.begin(), SplitOuter.end());
    else result.assign(Seven.begin(), profile.layout == SegmentLayout::Seven ? Seven.end() : Seven.end() - 1);
    if (profile.layout != SegmentLayout::Seven) result.insert(result.end(), Inner.begin(), Inner.end());
    const float gap = amount(profile.segmentGap) * .12f;
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

SegmentDisplayProfile normalized(SegmentDisplayProfile profile) {
    for (const auto field : {&SegmentDisplayProfile::thickness, &SegmentDisplayProfile::contrast, &SegmentDisplayProfile::glow,
            &SegmentDisplayProfile::cellSize, &SegmentDisplayProfile::horizontalGap, &SegmentDisplayProfile::verticalGap,
            &SegmentDisplayProfile::segmentGap, &SegmentDisplayProfile::brightness, &SegmentDisplayProfile::activationThreshold,
            &SegmentDisplayProfile::offSegmentVisibility, &SegmentDisplayProfile::blackDepth, &SegmentDisplayProfile::haloRadius})
        profile.*field = std::clamp(profile.*field, 0, SegmentDisplayProfile::MaximumIntensity);
    for (const auto field : {&SegmentDisplayProfile::responseTimeMilliseconds, &SegmentDisplayProfile::persistenceMilliseconds})
        profile.*field = std::clamp(profile.*field, 0, SegmentDisplayProfile::MaximumDurationMilliseconds);
    return profile;
}

void applySegmentDisplay(std::vector<LinearColor>& colors, QSize output, QSize source, const SegmentDisplayProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const int w = 6 + int(std::nearbyint(amount(profile.cellSize) * 26.f));
    const int h = std::max(9, int(std::nearbyint(w * 1.55f)));
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
    const float glow = amount(profile.glow), halo = 4.f / (.025f + amount(profile.haloRadius) * .24f);
    const float dim = amount(profile.offSegmentVisibility), brightness = .2f + amount(profile.brightness) * 1.3f;
    const float background = (1.f - amount(profile.blackDepth)) * .08f;
    const auto tint = Tints[size_t(profile.color)];
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
                const float d = distance(local, elements[i], profile.endShape);
                if (d < nearest) { nearest = d; active = i; }
            }
            const float t = unit((radius + edge - nearest) / (2.f * edge));
            const float core = t * t * (3.f - 2.f * t);
            const float spill = glow > 0 ? glow * std::exp(-std::max(0.f, nearest - radius) * halo) * (1.f - core) : 0.f;
            const auto cell = cellOffset(std::min(columns - 1, cellX), std::min(rows - 1, cellY));
            const float light = unit(std::max(dim, activations[cell + active]) * unit(core + spill) * brightness);
            auto& color = colors[size_t(y) * output.width() + x];
            for (size_t c = 0; c < color.size(); ++c) color[c] = unit(background + light * tint[c]);
        }
    }
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
