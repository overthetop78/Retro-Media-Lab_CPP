#include "CrtProcessing.h"
#include "CrtConnection.h"
#include <QVector2D>
#include <QVector3D>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr float Tau = std::numbers::pi_v<float> * 2.f;
constexpr float StripePeriod = 6.f, StripeWidth = 1.75f;
constexpr QVector3D Luma(.2126f, .7152f, .0722f);
constexpr QVector3D White(1.f, 1.f, 1.f);
constexpr int LowResolutionLines = 312;
constexpr int MaximumActivePalLines = 576, MaximumActiveNtscLines = 480;
constexpr int PalRasterLines = 625, NtscRasterLines = 525;
constexpr int PalRate = 50, NtscRate = 60, RateSwitch = 55;

float dot(QVector3D a, QVector3D b) { return QVector3D::dotProduct(a, b); }
template<class T>
T mix(T a, T b, float weight) { return a + (b - a) * weight; }
float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float fraction(float value) { return value - std::floor(value); }
bool onScreen(QVector2D uv) { return uv.x() >= 0 && uv.x() <= 1 && uv.y() >= 0 && uv.y() <= 1; }
size_t indexAt(int x, int y, QSize size) { return size_t(y) * size_t(size.width()) + size_t(x); }
QVector3D pixel(const std::vector<LinearColor>& colors, int x, int y, QSize size) {
    const auto& value = colors[indexAt(std::clamp(x, 0, size.width() - 1), std::clamp(y, 0, size.height() - 1), size)];
    return {value[0], value[1], value[2]};
}
QVector3D bilinear(const std::vector<LinearColor>& colors, QVector2D uv, QSize size) {
    const float x = uv.x() * float(size.width()) - .5f, y = uv.y() * float(size.height()) - .5f;
    const int left = int(std::floor(x)), top = int(std::floor(y));
    const auto a = mix(pixel(colors, left, top, size), pixel(colors, left + 1, top, size), x - float(left));
    const auto b = mix(pixel(colors, left, top + 1, size), pixel(colors, left + 1, top + 1, size), x - float(left));
    return mix(a, b, y - float(top));
}

}

CrtSignal::CrtSignal(const DisplaySignalProfile& profile, const Emulation::VideoFrame& frame, float aspect,
    int beamWidth, int screenCurvature) {
    using Emulation::VideoStandard;
    const auto standard = crtSourceStandard(frame);
    auto rate = frame.signalFieldRate;
    const bool source50 = rate > 0 ? rate < RateSwitch : standard != VideoStandard::Ntsc;
    if (!(rate > 0) || !std::isfinite(rate)) rate = source50 ? PalRate : NtscRate;
    const bool receiver50 = profile.fieldRate == CrtFieldRate::Hz50 ||
        (profile.fieldRate == CrtFieldRate::Hz50And60 && source50);
    const int receiverRate = receiver50 ? PalRate : NtscRate;
    const int active = receiver50 ? MaximumActivePalLines : MaximumActiveNtscLines;
    const int total = receiver50 ? PalRasterLines : NtscRasterLines;
    const int sourceLines = frame.signalFieldHeight > 0 ? frame.signalFieldHeight : frame.height;
    halfLines = float(active) * .5f;
    verticalScale = (sourceLines <= LowResolutionLines ? halfLines : float(active)) / float(std::max(1, sourceLines));
    aperture = float(active) / float(total);
    grid = QVector2D(float(active) * aspect, float(active)) * 3.f;
    const bool unlocked = source50 != receiver50;
    ratio = unlocked ? float(rate / receiverRate) : 0.f;
    const double drift = unlocked ? (double(frame.sequence) / rate) * (rate - receiverRate) : 0.;
    phase = float(drift - std::floor(drift));
    const int sourceTotal = source50 ? PalRasterLines : NtscRasterLines;
    lineSlip = unlocked ? float(sourceTotal * rate / (total * receiverRate) - 1.) : 0.f;
    curvature = .16f * float(screenCurvature) / CrtProfile::MaximumCurvature;
    exponent = std::exp2(float(beamWidth) / 10.f);
    const auto format = standard == VideoStandard::Pal ? CrtColorStandards::Pal :
        standard == VideoStandard::Secam ? CrtColorStandards::Secam : CrtColorStandards::Ntsc;
    const bool supportsColor = (int(profile.colorStandards) & int(format)) != 0;
    decoder = CrtDecoder::Separate;
    if (profile.connection == CrtConnection::Composite || profile.connection == CrtConnection::AnalogRf)
        decoder = !supportsColor ? CrtDecoder::Monochrome : standard == VideoStandard::Pal ? CrtDecoder::Pal :
            standard == VideoStandard::Secam ? CrtDecoder::Secam : CrtDecoder::Ntsc;
    if (profile.connection == CrtConnection::SVideo && !supportsColor) decoder = CrtDecoder::Monochrome;
}

QVector2D CrtSignal::screen(QVector2D uv) const {
    const auto delta = uv - QVector2D(.5f, .5f);
    return uv + delta * (4.f * curvature * delta.lengthSquared());
}
QVector2D CrtSignal::footprint(QVector2D uv, QVector2D pixelSize) const {
    const auto delta = uv - QVector2D(.5f, .5f);
    const float radial = 1.f + 4.f * curvature * delta.lengthSquared();
    const auto diagonal = QVector2D(radial, radial) + 8.f * curvature * delta * delta;
    return diagonal * pixelSize + 8.f * curvature * std::abs(delta.x() * delta.y()) * QVector2D(pixelSize.y(), pixelSize.x());
}
QVector2D CrtSignal::source(QVector2D uv) const {
    if (ratio <= 0.f) return uv;
    const float time = fraction((uv.y() - .5f) * aperture * ratio + .5f + phase);
    const float sourceAperture = ratio > 1.f ? float(MaximumActiveNtscLines) / NtscRasterLines :
        float(MaximumActivePalLines) / PalRasterLines;
    if (std::abs(time - .5f) > sourceAperture * .5f) return {-1.f, -1.f};
    const auto retrace = time * halfLines / aperture;
    return {uv.x() - .20f * std::sin(Tau * (retrace * lineSlip + phase)),
        (time - .5f) / sourceAperture * verticalScale + .5f};
}
float CrtSignal::beam(float y, int height, CrtMask mask) const {
    if (mask == CrtMask::ApertureGrille) return 1.f;
    const auto strength = .18f * unit(float(height) / halfLines - 1.f);
    const auto wave = .5f + .5f * std::cos(Tau * (y * halfLines - .5f));
    return 1.f - strength * (1.f - std::pow(unit(wave), exponent));
}

namespace {
QVector3D decode(QVector3D center, QVector3D neighbor, CrtDecoder decoder, int line) {
    if (decoder == CrtDecoder::Separate) return center;
    const auto gray = dot(center, Luma) * White;
    if (decoder == CrtDecoder::Monochrome) return gray;
    auto chroma = center - gray;
    const auto delay = neighbor - dot(neighbor, Luma) * White;
    if (decoder == CrtDecoder::Ntsc) chroma = mix(chroma, delay, .45f);
    if (decoder == CrtDecoder::Pal) chroma = mix(chroma, delay, .25f);
    if (decoder == CrtDecoder::Secam) {
        if ((line & 1) == 0) chroma.setZ(delay.z());
        else chroma.setX(delay.x());
        chroma.setY(-(Luma.x() * chroma.x() + Luma.z() * chroma.z()) / Luma.y());
    }
    const auto result = gray + chroma;
    return {unit(result.x()), unit(result.y()), unit(result.z())};
}
float opening(float position, float footprint, float period, float center, float radius) {
    const float start = position - center + radius, length = radius * 2.f;
    if (footprint <= 0.f) return period * fraction(start / period) < length ? 1.f : 0.f;
    const auto integral = [period, length](float coordinate) {
        const auto cycles = std::floor(coordinate / period);
        return cycles * length + std::clamp(coordinate - cycles * period, 0.f, length);
    };
    return (integral(start + footprint * .5f) - integral(start - footprint * .5f)) / footprint;
}
QVector3D stripes(float x, float footprint, float width = StripeWidth, float period = StripePeriod, float spacing = StripeWidth) {
    const auto stripe = [&](float offset) { return opening(x, footprint, period, offset + width * .5f, width * .5f); };
    return {stripe(0.f), stripe(spacing), stripe(2.f * spacing)};
}
QVector3D coverage(QVector2D point, QVector2D footprint, CrtMask mask) {
    if (mask == CrtMask::ApertureGrille) return stripes(point.x(), footprint.x(), StripeWidth, StripePeriod, StripePeriod / 3.f);
    if (mask == CrtMask::SlotMask) return stripes(point.x(), footprint.x());
    if (mask == CrtMask::StaggeredSlotMask) {
        const auto first = opening(point.y(), footprint.y(), 10.f, 5.f, 4.7f);
        const auto second = opening(point.y(), footprint.y(), 10.f, 0.f, 4.7f);
        return first * stripes(point.x(), footprint.x(), 1.85f, 2.f * StripePeriod, 1.85f) +
            second * stripes(point.x() - StripePeriod, footprint.x(), 1.85f, 2.f * StripePeriod, 1.85f);
    }
    const bool enhanced = mask == CrtMask::HitachiEdp;
    const QVector2D pitch = enhanced ? QVector2D(6.f, 7.f) : QVector2D(8.f, 6.f);
    const QVector2D radius = enhanced ? QVector2D(1.4f, 2.55f) : QVector2D(1.85f, 1.85f);
    const auto edge = [](float pos, float width, float period, float center, float r) {
        return .65f * opening(pos, width, period, center, r) +
            .35f * opening(pos, width, period, center, r * .7f);
    };
    const auto phosphor = [&](float x, float y) {
        return edge(point.x(), footprint.x(), pitch.x(), pitch.x() * x, radius.x()) *
                edge(point.y(), footprint.y(), pitch.y() * 2.f, y, radius.y()) +
            edge(point.x(), footprint.x(), pitch.x(), pitch.x() * (x + .5f), radius.x()) *
                edge(point.y(), footprint.y(), pitch.y() * 2.f, y + pitch.y(), radius.y());
    };
    return {phosphor(.25f, 2.f), phosphor(.75f, 2.f), phosphor(.5f, pitch.y() - 1.5f)};
}
QVector3D phosphorTint(CrtPhosphor phosphor) {
    if (phosphor == CrtPhosphor::Green) return {.16f, 1.f, .23f};
    if (phosphor == CrtPhosphor::Amber) return {1.f, .43f, .015f};
    return White;
}

LinearColor store(QVector3D color) { return {color.x(), color.y(), color.z()}; }
void applyMask(std::vector<LinearColor>& colors, QSize size, const CrtSignal& raster, const CrtProfile& profile, float gain) {
    if (profile.mask == CrtMask::None) return;
    const auto source = colors;
    const auto pitch = QVector2D(1.f / raster.grid.x(), 1.f / raster.grid.y());
    const QVector2D pixelSize(1.f / float(size.width()), 1.f / float(size.height()));
    const QVector2D across(pitch.x(), 0.f), down(0.f, pitch.y());
    const auto spread = std::clamp(.72f * std::sqrt(gain) / (1.f + raster.exponent * raster.exponent), 0.f, .78f);
    const auto tint = phosphorTint(profile.phosphor);
    const auto emitColor = [&](QVector3D color, QVector2D point, QVector2D footprint) {
        return profile.mask == CrtMask::Monochrome ? dot(color, Luma) * tint :
            color * coverage(point * raster.grid, footprint, profile.mask);
    };
    const auto sample = [&](QVector2D uv, QVector2D footprint) -> QVector3D {
        if (!onScreen(uv) || uv.x() == 1.f || uv.y() == 1.f) return {};
        const auto screen = raster.screen(uv);
        if (!onScreen(screen)) return {};
        return emitColor(pixel(source, int(uv.x() * float(size.width())), int(uv.y() * float(size.height())), size),
            screen, footprint);
    };
    for (int y = 0; y < size.height(); ++y)
        for (int x = 0; x < size.width(); ++x) {
            const QVector2D uv((float(x) + .5f) * pixelSize.x(), (float(y) + .5f) * pixelSize.y());
            const auto screen = raster.screen(uv);
            if (!onScreen(screen)) {
                colors[indexAt(x, y, size)] = {};
                continue;
            }
            const auto footprint = raster.grid * raster.footprint(uv, pixelSize);
            const auto direct = emitColor(pixel(source, x, y, size), screen, footprint);
            const auto neighbors = (sample(uv - across, footprint) + sample(uv + across, footprint) +
                sample(uv - down, footprint) + sample(uv + down, footprint)) * .25f;
            const auto glass = profile.mask == CrtMask::Monochrome ? 0.f :
                .00010f + .00075f * dot(coverage(screen * raster.grid, footprint, profile.mask), White);
            colors[indexAt(x, y, size)] = store(mix(direct, neighbors, spread) + glass * White);
        }
}
}

QImage filterPanelSignal(const QImage& input, const DisplaySignalProfile& panel, const Emulation::VideoFrame& frame) {
    if (input.isNull()) return input;
    const auto connected = filterCrtConnection(input, panel, frame);
    const CrtSignal raster(panel, frame, 1.f);
    if (raster.ratio <= 0.f && raster.decoder == CrtDecoder::Separate) return connected;
    QImage output(input.size(), QImage::Format_RGB32);
    if (output.isNull()) return connected;
    const int width = input.width(), height = input.height();
    const auto read = [&](int x, int y) {
        const auto rgb = connected.pixel(x, y);
        return QVector3D(qRed(rgb), qGreen(rgb), qBlue(rgb)) / 255.f;
    };
    for (int y = 0; y < height; ++y) {
        auto* line = reinterpret_cast<QRgb*>(output.scanLine(y));
        for (int x = 0; x < width; ++x) {
            const auto uv = raster.source({(x + .5f) / width, (y + .5f) / height});
            if (!onScreen(uv)) {
                line[x] = qRgb(0, 0, 0);
                continue;
            }
            const bool unlocked = raster.ratio > 0.f;
            const int sx = unlocked ? std::min(width - 1, int(uv.x() * width)) : x;
            const int sy = unlocked ? std::min(height - 1, int(uv.y() * height)) : y;
            const auto center = read(sx, sy);
            const auto delayed = raster.decoder == CrtDecoder::Ntsc ? read(std::max(0, sx - 1), sy) : read(sx, std::max(0, sy - 1));
            const auto color = decode(center, delayed, raster.decoder, sy);
            line[x] = qRgb(qRound(color.x() * 255.f), qRound(color.y() * 255.f), qRound(color.z() * 255.f));
        }
    }
    return output;
}

CrtShaderParameters crtShaderParameters(const CrtProfile& profile, const Emulation::VideoFrame& frame, float aspect, int brightness) {
    const CrtSignal signal(profile, frame, aspect, profile.beamWidth, profile.curvature);
    const auto tint = phosphorTint(profile.phosphor);
    return {{signal.verticalScale, signal.halfLines, signal.aperture, signal.phase},
        {signal.ratio, float(signal.decoder), signal.lineSlip, signal.grid.x() / 3.f},
        {tint.x(), tint.y(), tint.z(), float(profile.mask)},
        {signal.curvature, signal.exponent, std::exp2(float(brightness) / 10.f), 0.f}};
}

CrtProfile normalized(CrtProfile profile) {
    profile.curvature = std::clamp(profile.curvature, 0, CrtProfile::MaximumCurvature);
    profile.connectionDegradation = std::clamp(profile.connectionDegradation, 0, CrtProfile::MaximumConnectionDegradation);
    profile.beamWidth = std::clamp(profile.beamWidth, ImageAdjustments::Minimum, ImageAdjustments::Maximum);
    return profile;
}

void applyCrt(std::vector<LinearColor>& colors, QSize output, QSize sourceSize, const CrtProfile& profile,
        const CrtSignal& raster, float gain) {
    if (colors.empty() || output.isEmpty() || sourceSize.isEmpty()) return;
    const auto source = colors;
    const QVector2D neighbor = raster.decoder == CrtDecoder::Ntsc ? QVector2D(1.f / float(sourceSize.width()), 0.f)
        : QVector2D(0.f, 1.f / float(sourceSize.height()));
    for (int y = 0; y < output.height(); ++y)
        for (int x = 0; x < output.width(); ++x) {
            const auto screen = raster.screen({(float(x) + .5f) / float(output.width()),
                (float(y) + .5f) / float(output.height())});
            const auto uv = raster.source(screen);
            auto& result = colors[indexAt(x, y, output)];
            if (!onScreen(screen) || !onScreen(uv)) { result = {}; continue; }
            const auto center = bilinear(source, uv, output);
            const auto delayed = raster.decoder == CrtDecoder::Separate || raster.decoder == CrtDecoder::Monochrome
                ? center : bilinear(source, uv - neighbor, output);
            const auto color = decode(center, delayed, raster.decoder, int(uv.y() * float(sourceSize.height()))) *
                (gain * raster.beam(screen.y(), output.height(), profile.mask));
            result = {unit(color.x()), unit(color.y()), unit(color.z())};
        }
}

void applyCrtMask(std::vector<LinearColor>& colors, QSize output, const CrtProfile& profile,
        const CrtSignal& raster, float gain) {
    if (colors.empty() || output.isEmpty()) return;
    applyMask(colors, output, raster, profile, gain);
}
}
