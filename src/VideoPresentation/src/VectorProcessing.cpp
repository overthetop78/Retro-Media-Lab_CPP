#include "VectorProcessing.h"
#include "CrtProcessing.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
struct Phosphor {
    LinearColor tint;
    float halfLife, diffusion, focus;
};
constexpr std::array Phosphors{
    Phosphor{{}, 10.f, .10f, 1.f}, Phosphor{{.17f, 1.f, .12f}, 23.f, .15f, .91f},
    Phosphor{{1.f, .46f, .06f}, 33.f, .18f, 1.08f}, Phosphor{{1.f, 1.f, 1.f}, 12.f, .12f, 1.f}
};

float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float luma(const LinearColor& color) { return color[0] * .2126f + color[1] * .7152f + color[2] * .0722f; }
size_t index(QSize size, int x, int y) { return size_t(y) * size.width() + x; }

LinearColor tint(LinearColor color, const Phosphor& phosphor) {
    if (phosphor.tint == LinearColor{}) return color;
    const auto luminance = luma(color);
    for (size_t i = 0; i < color.size(); ++i) color[i] = luminance * phosphor.tint[i];
    return color;
}

void exciteRaster(std::vector<LinearColor>& emission, const std::vector<LinearColor>& source, QSize size,
        float beam, float sigma, float gain, const Phosphor& phosphor) {
    const int radius = std::clamp(int(std::ceil(sigma * 2.f)), 1, std::max(size.width(), size.height()));
    const float spread = std::min(.72f, .18f * beam);
    const float thinness = std::max(0.f, 1.f - beam);
    for (int y = 0; y < size.height(); ++y)
        for (int x = 0; x < size.width(); ++x) {
            const auto position = index(size, x, y);
            const auto& center = source[position];
            const auto& left = source[index(size, std::max(0, x - radius), y)];
            const auto& right = source[index(size, std::min(size.width() - 1, x + radius), y)];
            const auto& top = source[index(size, x, std::max(0, y - radius))];
            const auto& bottom = source[index(size, x, std::min(size.height() - 1, y + radius))];
            LinearColor neighbors{}, color{};
            for (size_t c = 0; c < color.size(); ++c) neighbors[c] = .25f * (left[c] + right[c] + top[c] + bottom[c]);
            const float attenuation = 1.f - thinness * (1.f - luma(center));
            const float diffusion = phosphor.diffusion * std::sqrt(beam) * unit(luma(neighbors) * gain);
            for (size_t c = 0; c < color.size(); ++c)
                color[c] = std::lerp(center[c], neighbors[c], spread) * attenuation + neighbors[c] * diffusion;
            color = tint(color, phosphor);
            for (size_t c = 0; c < color.size(); ++c) emission[position][c] += color[c] * gain;
        }
}

void exciteSegment(std::vector<LinearColor>& emission, const Emulation::VideoVectorSegment& segment, QSize size,
        float beam, float sigma, float gain, const Phosphor& phosphor) {
    const auto finite = [](const auto& values) {
        return std::ranges::all_of(values, [](float value) { return std::isfinite(value); });
    };
    if (!finite(segment.start) || !finite(segment.end) || !finite(segment.color) ||
        !std::isfinite(segment.intensity) || segment.intensity <= 0.f) return;
    const auto point = [size](const auto& coordinates) {
        return QVector2D(unit(coordinates[0]) * size.width() - .5f, unit(coordinates[1]) * size.height() - .5f);
    };
    const auto start = point(segment.start), end = point(segment.end), delta = end - start;
    const float length = delta.lengthSquared();
    auto color = segment.color;
    for (auto& channel : color) channel = unit(channel);
    color = tint(color, phosphor);
    for (auto& channel : color) channel *= gain * std::min(segment.intensity, 1.f);
    const float light = unit(luma(color));
    const float halo = phosphor.diffusion * std::sqrt(beam) * light;
    const float spread = sigma * (2.f + light);
    const float radius = (halo > .005f ? spread : sigma) * 3.f;
    const int firstRow = std::max(0, int(std::ceil(std::min(start.y(), end.y()) - radius)));
    const int lastRow = std::min(size.height() - 1, int(std::floor(std::max(start.y(), end.y()) + radius)));
    for (int y = firstRow; y <= lastRow; ++y) {
        const float lo = delta.y() == 0.f ? 0.f : unit((y - radius - start.y()) / delta.y());
        const float hi = delta.y() == 0.f ? 1.f : unit((y + radius - start.y()) / delta.y());
        const float a = start.x() + delta.x() * lo, b = start.x() + delta.x() * hi;
        const int first = std::max(0, int(std::ceil(std::min(a, b) - radius)));
        const int last = std::min(size.width() - 1, int(std::floor(std::max(a, b) + radius)));
        for (int x = first; x <= last; ++x) {
            const auto offset = QVector2D(x, y) - start;
            const float t = length > 0.f ? unit(QVector2D::dotProduct(offset, delta) / length) : 0.f;
            const float distance = (offset - t * delta).lengthSquared();
            const float energy = std::exp(-.5f * distance / (sigma * sigma)) + halo * std::exp(-.5f * distance / (spread * spread));
            auto& pixel = emission[index(size, x, y)];
            for (size_t c = 0; c < pixel.size(); ++c) pixel[c] += color[c] * energy;
        }
    }
}

LinearColor curvedPixel(const std::vector<LinearColor>& source, QSize size, QVector2D uv) {
    if (uv.x() < 0.f || uv.x() > 1.f || uv.y() < 0.f || uv.y() > 1.f) return {};
    const float x = std::clamp(uv.x() * size.width() - .5f, 0.f, float(size.width() - 1));
    const float y = std::clamp(uv.y() * size.height() - .5f, 0.f, float(size.height() - 1));
    const int left = int(x), top = int(y), right = std::min(left + 1, size.width() - 1), bottom = std::min(top + 1, size.height() - 1);
    LinearColor color{};
    for (size_t c = 0; c < color.size(); ++c)
        color[c] = std::lerp(std::lerp(source[index(size, left, top)][c], source[index(size, right, top)][c], x - left),
            std::lerp(source[index(size, left, bottom)][c], source[index(size, right, bottom)][c], x - left), y - top);
    return color;
}
}

VectorOptics vectorOptics(const VectorProfile& profile, QSize size) {
    const auto phosphor = Phosphors[size_t(profile.phosphorColor)];
    const float beam = phosphor.focus * std::exp2(float(profile.beamWidth) / 13.f);
    const float sigma = std::max(.30f, .62f * std::min(size.width(), size.height()) / 720.f * beam);
    return {phosphor.tint, beam, sigma, phosphor.diffusion, phosphor.halfLife};
}

VectorProfile normalized(VectorProfile profile) {
    profile.beamWidth = std::clamp(profile.beamWidth, VectorProfile::MinimumBeamWidth, VectorProfile::MaximumBeamWidth);
    profile.curvature = std::clamp(profile.curvature, 0, CrtProfile::MaximumCurvature);
    if (size_t(profile.phosphorColor) >= Phosphors.size()) profile.phosphorColor = VectorPhosphor::Color;
    return profile;
}

void renderVector(std::vector<LinearColor>& colors, QSize size, const VectorProfile& profile, int brightness,
        const Emulation::VideoFrame& frame, const Emulation::VideoVectorSegments& segments, VectorHistory& history) {
    const auto phosphor = Phosphors[size_t(profile.phosphorColor)];
    const bool native = bool(segments);
    const bool compatible = history.emission.size() == colors.size() && history.size == size &&
        history.profile.beamWidth == profile.beamWidth && history.profile.phosphorColor == profile.phosphorColor &&
        history.brightness == brightness && history.native == native && frame.sequence >= history.sequence &&
        frame.timestamp >= history.timestamp;
    if (!compatible || frame.sequence != history.sequence || frame.timestamp != history.timestamp) {
        if (compatible) {
            double elapsed = std::chrono::duration<double, std::milli>(frame.timestamp - history.timestamp).count();
            if (elapsed == 0.0) {
                const double rate = frame.signalFieldRate > 0 && frame.signalFieldRate < 1000 ? frame.signalFieldRate : 60.0;
                elapsed = double(frame.sequence - history.sequence) * 1000.0 / rate;
            }
            const float decay = std::exp2(-float(elapsed) / phosphor.halfLife);
            for (auto& pixel : history.emission)
                for (auto& channel : pixel) channel *= decay;
        } else {
            history.emission.assign(colors.size(), {});
        }
        const auto optics = vectorOptics(profile, size);
        const float beam = optics.scale, sigma = optics.sigma;
        const float gain = std::exp2(float(brightness) / 10.f);
        if (native) {
            for (const auto& segment : *segments) exciteSegment(history.emission, segment, size, beam, sigma, gain, phosphor);
        } else {
            exciteRaster(history.emission, colors, size, beam, sigma, gain, phosphor);
        }
        history.size = size;
        history.sequence = frame.sequence;
        history.timestamp = frame.timestamp;
        history.profile = profile;
        history.brightness = brightness;
        history.native = native;
    }

    if (!profile.curvature) {
        colors = history.emission;
        return;
    }
    const CrtSignal geometry({}, frame, 1.f, 0, profile.curvature);
    for (int y = 0; y < size.height(); ++y)
        for (int x = 0; x < size.width(); ++x) {
            const auto uv = geometry.screen({(x + .5f) / size.width(), (y + .5f) / size.height()});
            colors[index(size, x, y)] = curvedPixel(history.emission, size, uv);
        }
}
}
