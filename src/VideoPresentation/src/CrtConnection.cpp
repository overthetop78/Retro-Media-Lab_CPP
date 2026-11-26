#include "CrtConnection.h"
#include <QVector3D>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
using Emulation::VideoStandard;
constexpr QVector3D LumaWeights(.299f, .587f, .114f);
constexpr QVector3D White(1.f, 1.f, 1.f);
constexpr float ByteMaximum = 255.f;
constexpr double PalFieldRateCutoff = 55.;
constexpr std::array<QRgb, 16> CgaPalette{
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
};
constexpr std::array<QVector3D, 4> CarrierPhases{
    QVector3D(1.f, -.509f, 0.f), QVector3D(0.f, -.194f, 1.f),
    QVector3D(-1.f, .509f, 0.f), QVector3D(0.f, .194f, -1.f)
};

QVector3D rgb(QRgb pixel) { return QVector3D(float(qRed(pixel)), float(qGreen(pixel)), float(qBlue(pixel))) / ByteMaximum; }
QRgb packed(QVector3D color) {
    const auto toByte = [](float value) { return int(std::nearbyint(std::clamp(value, 0.f, 1.f) * ByteMaximum)); };
    return qRgb(toByte(color.x()), toByte(color.y()), toByte(color.z()));
}
float luma(QVector3D color) { return QVector3D::dotProduct(color, LumaWeights); }
QVector3D chroma(QVector3D color) { return color - luma(color) * White; }
QVector3D mix(QVector3D from, QVector3D to, float amount) { return from + (to - from) * amount; }

QVector3D quantize(QVector3D color, CrtMonitorPalette palette) {
    if (palette == CrtMonitorPalette::Mda) {
        const auto value = luma(color);
        return (value < .27f ? 0.f : value < .68f ? .57f : 1.f) * White;
    }
    if (palette == CrtMonitorPalette::Ega)
        return QVector3D(std::nearbyint(color.x() * 3.f), std::nearbyint(color.y() * 3.f),
            std::nearbyint(color.z() * 3.f)) / 3.f;
    QVector3D nearest;
    auto best = std::numeric_limits<float>::max();
    for (const auto value : CgaPalette) {
        const auto candidate = rgb(value);
        const auto distance = (color - candidate).lengthSquared();
        if (distance < best) {
            best = distance;
            nearest = candidate;
        }
    }
    return nearest;
}

float noise(quint32 x, quint32 y, quint32 frame) {
    quint32 value = (x * 374761393u + y * 668265263u + frame * 1446648779u) ^ 2246822519u;
    value = (value ^ (value >> 13)) * 1274126177u;
    value ^= value >> 16;
    return float(value & 65535u) * (2.f / 65535.f) - 1.f;
}

QVector3D sample(const QRgb* row, int width, float position) {
    const auto x = std::clamp(position, 0.f, float(width - 1));
    const int left = int(x);
    return mix(rgb(row[left]), rgb(row[std::min(left + 1, width - 1)]), x - float(left));
}

std::optional<VideoStandard> radioStandard(const DisplaySignalProfile& profile, const Emulation::VideoFrame& frame) {
    if (profile.connection != CrtConnection::AnalogRf) return {};
    switch (profile.colorStandards) {
    case CrtColorStandards::Pal: return VideoStandard::Pal;
    case CrtColorStandards::Secam: return VideoStandard::Secam;
    case CrtColorStandards::Ntsc: return VideoStandard::Ntsc;
    default: return crtSourceStandard(frame);
    }
}

struct AnalogParameters {
    float lumaLoss = 0, chromaLoss = 0, chromaRadius = 0, delay = 0, interference = 0;
};

AnalogParameters calculateParameters(CrtConnection connection, float d, std::optional<VideoStandard> standard) {
    switch (connection) {
    case CrtConnection::Component: return {.33f * d, .63f * d, 1.f + d, .55f * d, 0.f};
    case CrtConnection::SVideo: return {.08f + .37f * d, .28f + .5f * d, 1.15f + 1.5f * d, .12f + .48f * d, .008f + .027f * d};
    case CrtConnection::Composite: return {.19f + .55f * d, .58f + .38f * d, 1.5f + 2.f * d, .22f + .9f * d, .085f + .19f * d};
    case CrtConnection::AnalogRf: {
        const auto interference = standard == VideoStandard::Secam ? .018f + .048f * d
            : standard == VideoStandard::Pal ? .062f + .13f * d : .095f + .19f * d;
        return {.29f + .62f * d, .64f + .34f * d, 1.8f + 2.5f * d, .32f + d, interference};
    }
    default: return {};
    }
}

struct RadioLine {
    float noise = 0, burst = 0, hueSin = 0, hueCos = 1, secamLoss = 0;
    bool even = true;
};

RadioLine radioLine(int y, quint32 frame, float time, float d, std::optional<VideoStandard> standard) {
    RadioLine result;
    result.even = (y & 1) == 0;
    if (!standard) return result;
    const auto band = noise(quint32(y) / 5u, 1u, frame / 3u);
    result.noise = noise(quint32(y), 0, frame);
    result.burst = std::max(0.f, (band - .72f) / .28f);
    if (standard == VideoStandard::Secam) {
        result.secamLoss = std::clamp((std::abs(result.noise) - .55f) * (1.f + 2.f * d), 0.f, 1.f);
    } else {
        const auto hue = standard == VideoStandard::Pal ? (.035f + .25f * d) * band * (result.even ? 1.f : -1.f)
            : (.055f + .50f * d) * (.6f * std::sin(time) + .4f * band);
        result.hueSin = std::sin(hue);
        result.hueCos = std::cos(hue);
    }
    return result;
}

void applyRadio(float& light, QVector3D& color, const RadioLine& line, VideoStandard standard, float noiseValue, float d) {
    light += noiseValue * (.015f + .34f * d) + line.noise * (.012f + .085f * d) + line.burst * (.015f + .19f * d);
    color *= .91f - .38f * d + line.noise * .24f * d;
    if (standard == VideoStandard::Secam) {
        const auto disturbance = (line.noise * .09f + noiseValue * .045f) * d;
        if (line.even) color.setX(color.x() * (1.f - line.secamLoss) + disturbance);
        else color.setZ(color.z() * (1.f - line.secamLoss) + disturbance);
    } else {
        const auto red = color.x(), blue = color.z();
        color.setX(red * line.hueCos - blue * line.hueSin);
        color.setZ(red * line.hueSin + blue * line.hueCos);
    }
    color.setY(-(LumaWeights.x() * color.x() + LumaWeights.z() * color.z()) / LumaWeights.y());
}

void filterAnalog(const QImage& source, QImage& output, const DisplaySignalProfile& profile, const Emulation::VideoFrame& frame) {
    const auto standard = radioStandard(profile, frame);
    const float d = float(profile.connectionDegradation) / CrtProfile::MaximumConnectionDegradation;
    const auto params = calculateParameters(profile.connection, d, standard);
    const int width = source.width();
    const auto sequence = quint32(frame.sequence);
    const float time = float(sequence & 4095u) * .11f;
    const bool scart = profile.connection == CrtConnection::Scart;
    for (int y = 0; y < source.height(); ++y) {
        const auto* row = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        auto* dest = reinterpret_cast<QRgb*>(output.scanLine(y));
        const auto line = radioLine(y, sequence, time, d, standard);
        const auto offset = scart ? std::sin(float(y) * .048f + time) * .82f * d
            : standard ? std::sin(float(y) * .083f + time) * (.14f + 1.25f * d) + line.noise * .7f * d + line.burst * 2.4f * d
            : 0.f;
        for (int x = 0; x < width; ++x) {
            const auto position = float(x) + offset;
            const auto center = sample(row, width, position);
            const auto left = sample(row, width, position - 1.f), right = sample(row, width, position + 1.f);
            if (scart) {
                dest[x] = packed(mix(center, left * .58f + center * .24f + right * .18f, .85f * d));
                continue;
            }
            const auto light = luma(center);
            const auto averagedLight = luma(.5f * center + .25f * (left + right));
            auto outputLight = light + (averagedLight - light) * params.lumaLoss;
            const auto shifted = position - params.delay;
            const auto spread = chroma(.5f * sample(row, width, shifted) +
                .25f * (sample(row, width, shifted - params.chromaRadius) + sample(row, width, shifted + params.chromaRadius)));
            auto outputChroma = mix(chroma(center), spread, params.chromaLoss);
            if (params.interference > 0.f) {
                const auto phase = (quint32(x) + quint32(y) * 2u + (sequence & 3u)) & 3u;
                auto carrier = CarrierPhases[phase];
                if (standard == VideoStandard::Pal && !line.even && (phase & 1u) == 0) carrier = -carrier;
                outputChroma += carrier * ((light - averagedLight) * params.interference * 2.1f);
                outputLight += QVector3D::dotProduct(chroma(right - left), carrier) * params.interference * .48f;
            }
            if (profile.connection == CrtConnection::SVideo) outputChroma *= .96f - .27f * d;
            if (standard) applyRadio(outputLight, outputChroma, line, *standard, noise(quint32(x), quint32(y), sequence), d);
            dest[x] = packed(outputLight * White + outputChroma);
        }
    }
}
}

VideoStandard crtSourceStandard(const Emulation::VideoFrame& frame) {
    return frame.signalStandard.value_or(frame.signalFieldRate > 0 && frame.signalFieldRate < PalFieldRateCutoff
        ? VideoStandard::Pal : VideoStandard::Ntsc);
}

QImage filterCrtConnection(const QImage& input, const DisplaySignalProfile& profile, const Emulation::VideoFrame& frame) {
    if (input.isNull() || profile.connection == CrtConnection::Digital ||
        (profile.connectionDegradation == 0 && (profile.connection == CrtConnection::Scart ||
            profile.connection == CrtConnection::Component))) return input;
    QImage output(input.size(), QImage::Format_RGB32);
    if (output.isNull()) return input;
    if (profile.connection == CrtConnection::LegacyMonitor) {
        for (int y = 0; y < input.height(); ++y) {
            const auto* source = reinterpret_cast<const QRgb*>(input.constScanLine(y));
            auto* destination = reinterpret_cast<QRgb*>(output.scanLine(y));
            for (int x = 0; x < input.width(); ++x) destination[x] = packed(quantize(rgb(source[x]), profile.monitorPalette));
        }
    } else filterAnalog(input, output, profile, frame);
    return output;
}
}
