#include "RestorationProcessing.h"
#include "ColorTransfer.h"

#include <QVector3D>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr int PatternTolerance = 3;
constexpr int PatternContrast = 10;
constexpr std::array BandRadii{2, 4, 8, 16, 32};
constexpr float BandEdgeLimit = .075f;
constexpr float FlatThreshold = .0015f;
constexpr float DetailThreshold = .018f;
constexpr float ByteMaximum = 255.f;
constexpr QVector3D Luma{.2126f, .7152f, .0722f};

enum class Pattern { Checkerboard, Columns, Rows, PairedColumnsA, PairedColumnsB, PairedRowsA, PairedRowsB };

constexpr bool alternateAt(Pattern pattern, int x, int y) {
    switch (pattern) {
    case Pattern::Checkerboard: return ((x + y) & 1) != 0;
    case Pattern::Columns: return (x & 1) != 0;
    case Pattern::Rows: return (y & 1) != 0;
    case Pattern::PairedColumnsA: return ((x + 1) & 2) != 0;
    case Pattern::PairedColumnsB: return (x & 2) != 0;
    case Pattern::PairedRowsA: return ((y + 1) & 2) != 0;
    case Pattern::PairedRowsB: return (y & 2) != 0;
    }
    return false;
}

int distance(QRgb a, QRgb b) {
    return std::max({std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b))});
}

bool similar(QRgb a, QRgb b) { return distance(a, b) <= PatternTolerance; }

std::optional<std::pair<Pattern, QRgb>> identify(QRgb center, QRgb left, QRgb right, QRgb up, QRgb down) {
    if ((((center ^ left) | (center ^ right) | (center ^ up) | (center ^ down)) & 0x00ffffffu) == 0) return {};
    if (similar(left, right)) {
        if (similar(left, up) && similar(up, down)) return {{Pattern::Checkerboard, left}};
        if (similar(center, up) && similar(center, down)) return {{Pattern::Columns, left}};
    }
    const bool matchesLeft = similar(center, left), matchesRight = similar(center, right);
    const bool matchesUp = similar(center, up), matchesDown = similar(center, down);
    if (matchesLeft && matchesRight && similar(up, down)) return {{Pattern::Rows, up}};
    if (matchesUp && matchesDown && matchesLeft != matchesRight)
        return matchesLeft ? std::pair{Pattern::PairedColumnsA, right} : std::pair{Pattern::PairedColumnsB, left};
    if (matchesLeft && matchesRight && matchesUp != matchesDown)
        return matchesUp ? std::pair{Pattern::PairedRowsA, down} : std::pair{Pattern::PairedRowsB, up};
    return {};
}

bool matches(const QImage& source, int x, int y, QRgb center, QRgb alternate, Pattern pattern) {
    for (int dy = -2; dy <= 2; ++dy) {
        const auto* row = reinterpret_cast<const QRgb*>(source.constScanLine(y + dy));
        for (int dx = -2; dx <= 2; ++dx)
            if (!similar(row[x + dx], alternateAt(pattern, dx, dy) ? alternate : center)) return false;
    }
    return true;
}

float strength(int value) { return float(value) / ImageRestoration::MaximumIntensity; }
float fraction(float value) { return value - std::floor(value); }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float maxChannel(QVector3D value) { return std::max({std::abs(value.x()), std::abs(value.y()), std::abs(value.z())}); }

size_t indexAt(int x, int y, QSize size) { return size_t(y) * size.width() + x; }

QVector3D read(const std::vector<LinearColor>& colors, QSize size, int x, int y) {
    const auto& value = colors[indexAt(x, y, size)];
    return {value[0], value[1], value[2]};
}

void write(std::vector<LinearColor>& colors, QSize size, int x, int y, QVector3D value) {
    colors[indexAt(x, y, size)] = {unit(value.x()), unit(value.y()), unit(value.z())};
}

bool isBetween(QVector3D sample, QVector3D endpoint) {
    const float ratio = QVector3D::dotProduct(sample, endpoint) / endpoint.lengthSquared();
    return ratio >= -.08f && ratio <= 1.08f && maxChannel(sample - endpoint * ratio) <= FlatThreshold * 2.f;
}

std::optional<QVector3D> bandCorrection(const std::vector<LinearColor>& source, QSize size, int x, int y,
        int dx, int dy, QVector3D center) {
    const int available = dx ? std::min(x, size.width() - 1 - x) : std::min(y, size.height() - 1 - y);
    const auto sample = [&](int distance) { return read(source, size, x + dx * distance, y + dy * distance) - center; };
    const auto firstStep = [&](int sign, int far, QVector3D delta) {
        int near = 0;
        const float threshold = delta.lengthSquared() * .5f;
        for (int i = 0; i < 5 && far - near > 1; ++i) {
            const int middle = (near + far) / 2;
            if (QVector3D::dotProduct(sample(sign * middle), delta) >= threshold) far = middle;
            else near = middle;
        }
        return far;
    };
    for (int radius : BandRadii) {
        if (radius > available) break;
        const auto before = sample(-radius), after = sample(radius);
        const float smaller = std::min(maxChannel(before), maxChannel(after));
        const float span = std::max(maxChannel(before), maxChannel(after));
        if (span > BandEdgeLimit) return {};
        if (smaller < std::max(FlatThreshold, span * .25f)) continue;
        if (QVector3D::dotProduct(before, after) >= -.85f * before.length() * after.length() ||
            !isBetween(sample(-radius / 2), before) || !isBetween(sample(radius / 2), after)) return {};
        const int left = firstStep(-1, radius, before), right = firstStep(1, radius, after);
        return (before + (after - before) * ((left - .5f) / (left + right - 1.f))) * .5f;
    }
    return {};
}

QVector3D lostDetail(QVector3D center, QVector3D filtered, QVector3D before, QVector3D after) {
    const auto detail = center - (before + after) * .5f;
    const float amplitude = maxChannel(detail);
    if (amplitude < DetailThreshold || maxChannel(before - after) > amplitude * .45f) return {};
    const float missing = unit(QVector3D::dotProduct(center - filtered, detail) / detail.lengthSquared());
    auto bounded = filtered + detail * missing;
    for (int channel = 0; channel < 3; ++channel)
        bounded[channel] = std::clamp(bounded[channel], std::min(center[channel], filtered[channel]),
            std::max(center[channel], filtered[channel]));
    return bounded - filtered;
}
}

ImageRestoration normalized(ImageRestoration value) {
    for (const auto field : {&ImageRestoration::dedithering, &ImageRestoration::debanding, &ImageRestoration::detailRecovery})
        value.*field = std::clamp(value.*field, 0, ImageRestoration::MaximumIntensity);
    return value;
}

QImage dedither(const QImage& source, int intensity) {
    if (intensity <= 0 || source.width() < 5 || source.height() < 5) return source;
    const float weight = strength(intensity) * .5f;
    static const auto values = [] {
        std::array<float, 256> lookup{};
        for (size_t level = 0; level < lookup.size(); ++level) lookup[level] = linear(float(level) / ByteMaximum);
        return lookup;
    }();
    const auto mix = [&](int a, int b) { return int(std::nearbyint(srgb(std::lerp(values[a], values[b], weight)) * ByteMaximum)); };
    QImage output = source;
    for (int y = 2; y < source.height() - 2; ++y) {
        const auto* row = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        const auto* up = reinterpret_cast<const QRgb*>(source.constScanLine(y - 1));
        const auto* down = reinterpret_cast<const QRgb*>(source.constScanLine(y + 1));
        QRgb* target = nullptr;
        for (int x = 2; x < source.width() - 2; ++x) {
            const auto center = row[x];
            const auto candidate = identify(center, row[x - 1], row[x + 1], up[x], down[x]);
            if (!candidate) continue;
            const auto [pattern, alternate] = *candidate;
            if (distance(center, alternate) < PatternContrast || !matches(source, x, y, center, alternate, pattern)) continue;
            if (!target) target = reinterpret_cast<QRgb*>(output.scanLine(y));
            target[x] = qRgba(mix(qRed(center), qRed(alternate)), mix(qGreen(center), qGreen(alternate)),
                mix(qBlue(center), qBlue(alternate)), qAlpha(center));
        }
    }
    return output;
}

void deband(std::vector<LinearColor>& colors, QSize size, int intensity) {
    if (intensity <= 0 || size.width() < 5 || size.height() < 5) return;
    const auto source = colors;
    const float amount = strength(intensity);
    for (int y = 1; y < size.height() - 1; ++y)
        for (int x = 1; x < size.width() - 1; ++x) {
            const auto center = read(source, size, x, y);
            if (maxChannel(read(source, size, x - 1, y) - center) > BandEdgeLimit ||
                maxChannel(read(source, size, x + 1, y) - center) > BandEdgeLimit ||
                maxChannel(read(source, size, x, y - 1) - center) > BandEdgeLimit ||
                maxChannel(read(source, size, x, y + 1) - center) > BandEdgeLimit) continue;
            auto correction = bandCorrection(source, size, x, y, 1, 0, center);
            if (!correction) correction = bandCorrection(source, size, x, y, 0, 1, center);
            if (!correction) continue;
            const float noise = fraction(52.9829189f * fraction(.06711056f * x + .00583715f * y)) - .5f;
            const float step = (.08f + 2.f * std::sqrt(unit(QVector3D::dotProduct(center, Luma)))) / ByteMaximum;
            const float offset = noise * step;
            write(colors, size, x, y, center + (*correction + QVector3D(offset, offset, offset)) * amount);
        }
}

void recoverDetail(const std::vector<LinearColor>& reference, QSize source, std::vector<LinearColor>& colors,
        QSize output, int intensity) {
    if (intensity <= 0 || source.width() < 3 || source.height() < 3) return;
    const float amount = strength(intensity);
    for (int y = 0; y < output.height(); ++y) {
        const float sy = (y + .5f) / output.height() * source.height();
        const int iy = int(sy);
        if (iy <= 0 || iy >= source.height() - 1) continue;
        for (int x = 0; x < output.width(); ++x) {
            const float sx = (x + .5f) / output.width() * source.width();
            const int ix = int(sx);
            if (ix <= 0 || ix >= source.width() - 1) continue;
            const auto center = read(reference, source, ix, iy), filtered = read(colors, output, x, y);
            const auto direction = [&](int dx, int dy, int available) {
                auto before = read(reference, source, ix - dx, iy - dy);
                auto after = read(reference, source, ix + dx, iy + dy);
                if (available >= 2) {
                    if (maxChannel(center - before) <= FlatThreshold && maxChannel(center - after) >= DetailThreshold)
                        before = read(reference, source, ix - 2 * dx, iy - 2 * dy);
                    if (maxChannel(center - after) <= FlatThreshold && maxChannel(center - before) >= DetailThreshold)
                        after = read(reference, source, ix + 2 * dx, iy + 2 * dy);
                }
                return lostDetail(center, filtered, before, after);
            };
            const auto horizontal = direction(1, 0, std::min(ix, source.width() - 1 - ix));
            const auto vertical = direction(0, 1, std::min(iy, source.height() - 1 - iy));
            const auto detail = horizontal.lengthSquared() >= vertical.lengthSquared() ? horizontal : vertical;
            const float distance = std::max(std::abs(sx - ix - .5f), std::abs(sy - iy - .5f));
            write(colors, output, x, y, filtered + detail * (unit((.5f - distance) * 4.f) * amount));
        }
    }
}
}
