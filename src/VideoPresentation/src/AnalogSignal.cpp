#include "AnalogSignal.h"
#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr float RedWeight = .299f, GreenWeight = .587f, BlueWeight = .114f;
constexpr float ScartDegradation = .14f, ComponentDegradation = .34f, SVideoDegradation = .52f;
constexpr int CompositeRadius = 2;

struct Yiq {
    float luminance = 0, inPhase = 0, quadrature = 0;
};

float bounded(float value) { return std::clamp(value, 0.f, 1.f); }
float luminance(const LinearColor& rgb) { return rgb[0] * RedWeight + rgb[1] * GreenWeight + rgb[2] * BlueWeight; }
Yiq components(const LinearColor& rgb) {
    return {luminance(rgb), rgb[0] * .596f - rgb[1] * .274f - rgb[2] * .322f,
        rgb[0] * .211f - rgb[1] * .523f + rgb[2] * .312f};
}
LinearColor fromYiq(Yiq c) {
    return {bounded(c.luminance + .956f * c.inPhase + .621f * c.quadrature),
        bounded(c.luminance - .272f * c.inPhase - .647f * c.quadrature),
        bounded(c.luminance - 1.106f * c.inPhase + 1.703f * c.quadrature)};
}
size_t position(int x, int y, int width) { return size_t(y) * size_t(width) + size_t(x); }

LinearColor rgbScart(const LinearColor& center, const LinearColor& left, float amount) {
    LinearColor result{};
    for (size_t i = 0; i < result.size(); ++i) result[i] = std::lerp(center[i], left[i], amount * ScartDegradation);
    return result;
}
LinearColor chromaBlur(Yiq center, Yiq left, Yiq right, float amount, SignalConnection connection) {
    const bool sVideo = connection == SignalConnection::SVideo;
    const auto average = [sVideo](float a, float b, float c) { return sVideo ? (a + b + c) / 3.f : (a + c) * .5f; };
    const float strength = amount * (sVideo ? SVideoDegradation : ComponentDegradation);
    center.inPhase = std::lerp(center.inPhase, average(left.inPhase, center.inPhase, right.inPhase), strength);
    center.quadrature = std::lerp(center.quadrature, average(left.quadrature, center.quadrature, right.quadrature), strength);
    return fromYiq(center);
}
LinearColor composite(const std::vector<LinearColor>& source, Yiq center, int width, int x, int y, quint64 sequence, float amount) {
    Yiq sum;
    float lumaWeight = 0;
    for (int offset = -CompositeRadius; offset <= CompositeRadius; ++offset) {
        const auto sample = components(source[position(std::clamp(x + offset, 0, width - 1), y, width)]);
        if (std::abs(offset) <= 1) {
            const float weight = offset == 0 ? 2.f : 1.f;
            sum.luminance += sample.luminance * weight;
            lumaWeight += weight;
        }
        sum.inPhase += sample.inPhase;
        sum.quadrature += sample.quadrature;
    }
    constexpr float ChromaSamples = CompositeRadius * 2 + 1;
    const auto crawl = ((quint64(x) + quint64(y) + sequence) & 1 ? -1.f : 1.f) * amount * .10f;
    return fromYiq({std::lerp(center.luminance, sum.luminance / lumaWeight, amount * .62f),
        std::lerp(center.inPhase, sum.inPhase / ChromaSamples, amount * .92f) + crawl,
        std::lerp(center.quadrature, sum.quadrature / ChromaSamples, amount * .92f) - crawl});
}
LinearColor radioFrequency(const LinearColor& center, const LinearColor& left, const LinearColor& right,
        int x, int y, quint64 sequence, float amount, SignalStandard standard) {
    quint32 hash = (quint32(x) * 374761393u + quint32(y) * 668265263u) ^ (quint32(sequence) * 2246822519u);
    hash = (hash ^ (hash >> 13)) * 1274126177u;
    const auto noise = (float(hash & 1023u) / 511.5f - 1.f) * amount;
    const auto interference = std::sin(float(y) * .41f + noise * 23.f) * amount * .055f;
    const auto defect = [&]() -> LinearColor {
        switch (standard) {
        case SignalStandard::Ntsc: return {noise * .18f + interference, -noise * .04f, noise * .10f - interference * .55f};
        case SignalStandard::Secam: return {noise * .10f + interference, noise * .06f - interference * .35f, -noise * .15f - interference};
        default: return {noise * .16f + interference, noise * .10f - interference * .25f, noise * .13f - interference};
        }
    }();
    LinearColor result{};
    for (size_t i = 0; i < result.size(); ++i) {
        const auto blur = (left[i] + center[i] * 2.f + right[i]) * .25f;
        result[i] = bounded(std::lerp(center[i], blur, amount * .92f) + defect[i]);
    }
    return result;
}
void simulateConnection(std::vector<LinearColor>& image, int width, int height, quint64 sequence, SignalSimulation signal) {
    if (signal.connectionIntensity <= 0 || signal.connection == SignalConnection::None ||
        (width < 2 && signal.connection != SignalConnection::Rf)) return;
    const auto source = image;
    const auto amount = float(signal.connectionIntensity) / SignalSimulation::MaximumIntensity;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const auto index = position(x, y, width);
            const auto& center = source[index];
            const auto& left = source[position(std::max(0, x - 1), y, width)];
            const auto& right = source[position(std::min(width - 1, x + 1), y, width)];
            switch (signal.connection) {
            case SignalConnection::None: break;
            case SignalConnection::RgbScart: image[index] = rgbScart(center, left, amount); break;
            case SignalConnection::Component:
            case SignalConnection::SVideo:
                image[index] = chromaBlur(components(center), components(left), components(right), amount, signal.connection);
                break;
            case SignalConnection::Composite:
                image[index] = composite(source, components(center), width, x, y, sequence, amount);
                break;
            case SignalConnection::Rf:
                image[index] = radioFrequency(center, left, right, x, y, sequence, amount, signal.standard);
                break;
            }
        }
}
LinearColor pal(Yiq current, Yiq partner, float amount) {
    current.inPhase = std::lerp(current.inPhase, partner.inPhase, amount * .58f);
    current.quadrature = std::lerp(current.quadrature, partner.quadrature, amount * .72f);
    return fromYiq(current);
}
LinearColor ntsc(Yiq current, Yiq delayed, float amount) {
    current.luminance = std::lerp(current.luminance, delayed.luminance, amount * .24f);
    current.inPhase = std::lerp(current.inPhase, delayed.inPhase, amount * .72f);
    current.quadrature = std::lerp(current.quadrature, delayed.quadrature, amount * .84f);
    const auto hueError = amount * .12f;
    const auto rotated = current.inPhase - current.quadrature * hueError;
    current.quadrature += current.inPhase * hueError;
    current.inPhase = rotated;
    return fromYiq(current);
}
LinearColor secam(const LinearColor& current, const LinearColor& previous, bool odd, float amount) {
    const float light = luminance(current), previousLight = luminance(previous);
    const auto red = bounded(light + (odd ? current[0] - light
        : std::lerp(current[0] - light, previous[0] - previousLight, amount)));
    const auto blue = bounded(light + (odd ? std::lerp(current[2] - light, previous[2] - previousLight, amount)
        : current[2] - light));
    return {red, bounded((light - RedWeight * red - BlueWeight * blue) / GreenWeight), blue};
}
void simulateStandard(std::vector<LinearColor>& image, int width, int height, SignalSimulation signal) {
    if (signal.standardIntensity <= 0 || signal.standard == SignalStandard::Automatic ||
        (signal.standard == SignalStandard::Ntsc ? width < 2 : height < 2)) return;
    const auto source = image;
    const auto amount = float(signal.standardIntensity) / SignalSimulation::MaximumIntensity;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const auto index = position(x, y, width);
            const auto& current = source[index];
            switch (signal.standard) {
            case SignalStandard::Automatic: break;
            case SignalStandard::Pal: {
                const auto neighbor = std::clamp(y + ((y & 1) ? -1 : 1), 0, height - 1);
                image[index] = pal(components(current), components(source[position(x, neighbor, width)]), amount);
                break;
            }
            case SignalStandard::Ntsc:
                image[index] = ntsc(components(current), components(source[position(std::max(0, x - 1), y, width)]), amount);
                break;
            case SignalStandard::Secam:
                image[index] = secam(current, source[position(x, std::max(0, y - 1), width)], (y & 1) != 0, amount * .88f);
                break;
            }
        }
}
}

SignalSimulation normalized(SignalSimulation signal) {
    signal.connectionIntensity = std::clamp(signal.connectionIntensity, 0, SignalSimulation::MaximumIntensity);
    signal.standardIntensity = std::clamp(signal.standardIntensity, 0, SignalSimulation::MaximumIntensity);
    return signal;
}

bool hasSignalEffect(SignalSimulation signal) {
    return (signal.connection != SignalConnection::None && signal.connectionIntensity > 0) || signal.standardIntensity > 0;
}

void simulateSignal(std::vector<LinearColor>& image, int width, int height, quint64 sequence, SignalSimulation signal) {
    simulateConnection(image, width, height, sequence, signal);
    simulateStandard(image, width, height, signal);
}
}
