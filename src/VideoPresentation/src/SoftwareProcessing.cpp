#include "SoftwareProcessing.h"
#include "SamplingFilters.h"
#include "CrtProcessing.h"
#include "FixedPixelProcessing.h"
#include "RestorationProcessing.h"
#include "LedMatrixProcessing.h"
#include "VfdProcessing.h"
#include "DotMatrixProcessing.h"
#include "PlasmaProcessing.h"
#include "VectorProcessing.h"
#include "ColorTransfer.h"
#include "CrtConnection.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr int MotionTileWidth = 4;
constexpr int MotionTileHeight = 2;
constexpr int MaximumBobOffset = 2;
constexpr int BobOffsetPenalty = 7;
constexpr int ColorMaximum = 255;

int luma(QRgb pixel) { return (54 * qRed(pixel) + 183 * qGreen(pixel) + 19 * qBlue(pixel)) >> 8; }
int distance(QRgb a, QRgb b) {
    const auto channel = std::max({std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b))});
    return (std::abs(luma(a) - luma(b)) + channel) / 2;
}
float ramp(float value, float low, float high) {
    return std::clamp((value - low) / (high - low), 0.0f, 1.0f);
}
QRgb mix(QRgb a, QRgb b, float weight) {
    const auto channel = [weight](int first, int second) { return int(std::nearbyint(std::lerp(float(first), float(second), weight))); };
    return qRgb(channel(qRed(a), qRed(b)), channel(qGreen(a), qGreen(b)), channel(qBlue(a), qBlue(b)));
}
QRgb quantize(QRgb value, Emulation::PixelFormat format) {
    if (format == Emulation::PixelFormat::Xrgb8888) return value;
    constexpr int Maximum5Bits = 31, Maximum6Bits = 63;
    const auto channel = [](int component, int maximum) {
        return ((component * maximum + ColorMaximum / 2) / ColorMaximum) * ColorMaximum / maximum;
    };
    return qRgb(channel(qRed(value), Maximum5Bits),
        channel(qGreen(value), format == Emulation::PixelFormat::Rgb565 ? Maximum6Bits : Maximum5Bits),
        channel(qBlue(value), Maximum5Bits));
}
QRgb bob(const QImage& field, int x, int row, int parity) {
    const int upper = row - parity;
    const int lower = upper + 1;
    if (upper < 0 || lower >= field.height()) return field.pixel(x, std::clamp(upper, 0, field.height() - 1));
    const auto score = [&](int offset) {
        int result = 2 * distance(field.pixel(x + offset, upper), field.pixel(x - offset, lower));
        for (const int dx : {-1, 1}) {
            const int a = std::clamp(x + offset + dx, 0, field.width() - 1);
            const int b = std::clamp(x - offset + dx, 0, field.width() - 1);
            result += distance(field.pixel(a, upper), field.pixel(b, lower));
        }
        return result;
    };
    int bestOffset = 0;
    int bestScore = score(0);
    for (int offset = -MaximumBobOffset; offset <= MaximumBobOffset; ++offset) {
        const auto magnitude = std::abs(offset);
        if (!magnitude || x - magnitude < 0 || x + magnitude >= field.width()) continue;
        const auto candidate = score(offset) + magnitude * BobOffsetPenalty;
        if (candidate < bestScore) {
            bestOffset = offset;
            bestScore = candidate;
        }
    }
    return mix(field.pixel(x + bestOffset, upper), field.pixel(x - bestOffset, lower), .5f);
}
void updateMotion(const QImage& current, const QImage& previous, const QImage& sameParity,
        const QImage& previousComplement, int parity, quint64 sequence, int tilesAcross,
        QList<float>& motion, std::optional<quint64>& motionSequence) {
    if (motionSequence == sequence) return;
    const auto smooth = motionSequence && sequence - *motionSequence == 1;
    motion.resize(qsizetype(tilesAcross) * ((current.height() + MotionTileHeight - 1) / MotionTileHeight));
    for (int y = 0; y < current.height(); y += MotionTileHeight)
        for (int x = 0; x < current.width(); x += MotionTileWidth) {
            int sum = 0, maximum = 0, samples = 0;
            const int bottom = std::min(y + MotionTileHeight, current.height());
            const int right = std::min(x + MotionTileWidth, current.width());
            for (int row = y; row < bottom; ++row)
                for (int column = x; column < right; ++column) {
                    const auto complement = previous.pixel(column, row);
                    auto change = sameParity.isNull() ? distance(complement, bob(current, column, row, parity))
                        : distance(current.pixel(column, row), sameParity.pixel(column, row));
                    if (!previousComplement.isNull())
                        change = std::max(change, distance(complement, previousComplement.pixel(column, row)));
                    sum += change;
                    maximum = std::max(maximum, change);
                    ++samples;
                }
            auto& tile = motion[qsizetype(y / MotionTileHeight) * tilesAcross + x / MotionTileWidth];
            const auto detected = ramp(.75f * float(sum) / float(samples) + .25f * float(maximum), 5, 25);
            tile = smooth ? std::lerp(tile, detected, detected < tile ? .36f : 1.f) : detected;
        }
    motionSequence = sequence;
}
void blendVertical(QImage& image, Emulation::PixelFormat format) {
    const auto source = image;
    for (int y = 1; y < image.height() - 1; ++y) {
        auto* output = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const auto a = source.pixel(x, y - 1);
            const auto c = source.pixel(x, y);
            const auto b = source.pixel(x, y + 1);
            const auto edge = distance(a, b);
            const auto alternating = std::min(distance(a, c), distance(c, b));
            const auto weight = ramp(float(alternating - edge), 4, 32) * (1.f - ramp(float(edge), 12, 52));
            if (weight > 0) output[x] = quantize(mix(c, mix(a, b, .5f), .5f * weight), format);
        }
    }
}
using Color = LinearColor;
float luminance(const Color& value) { return value[0] * .2126f + value[1] * .7152f + value[2] * .0722f; }
float progress(int value) { return float(value) / ImageAdjustments::Maximum; }
}

double frameAspectRatio(const Emulation::VideoFrame& frame, QSize fallback) {
    if (frame.aspectRatio > 0 && std::isfinite(frame.aspectRatio)) return frame.aspectRatio;
    if (frame.width <= 0 || frame.height <= 0)
        return double(std::max(1, fallback.width())) / std::max(1, fallback.height());
    const double imageHeight = frame.scanMode == Emulation::ScanMode::Progressive ? frame.height : 2.0 * frame.height;
    return double(frame.width) / imageHeight;
}

double screenAspectRatio(const Profile& profile, double imageAspect) {
    const auto choices = aspectRatioChoices(profile.screenType());
    if (std::ranges::find(choices, profile.aspectRatio) == choices.end()) return imageAspect;
    const auto dimensions = aspectRatioDimensions(profile.aspectRatio);
    return dimensions.isEmpty() ? imageAspect : double(dimensions.width()) / dimensions.height();
}

QImage composeFields(const QImage& current, const QImage& previous, const QImage& sameParity,
        const QImage& previousComplement, const Emulation::VideoFrame& frame, Deinterlacing mode,
        QList<float>& motion, std::optional<quint64>& motionSequence) {
    using Emulation::ScanMode;
    if (current.isNull() || frame.scanMode == ScanMode::Progressive || mode == Deinterlacing::Off ||
        current.height() > std::numeric_limits<int>::max() / 2) return current;
    const int parity = frame.scanMode == ScanMode::OddField ? 1 : 0;
    const bool complement = !previous.isNull() && mode != Deinterlacing::Bob;
    const bool adaptive = mode == Deinterlacing::Adaptive && complement;
    const int tilesAcross = (current.width() - 1) / MotionTileWidth + 1;
    if (adaptive) updateMotion(current, previous, sameParity, previousComplement, parity, frame.sequence, tilesAcross, motion, motionSequence);
    QImage output(current.width(), current.height() * 2, current.format());
    if (output.isNull()) return output;
    for (int y = 0; y < output.height(); ++y) {
        const int row = y / 2;
        const bool own = (y & 1) == parity;
        const auto& field = own || !complement ? current : previous;
        auto* destination = reinterpret_cast<QRgb*>(output.scanLine(y));
        std::copy_n(reinterpret_cast<const QRgb*>(field.constScanLine(row)), output.width(), destination);
        if (own || (complement && !adaptive)) continue;
        for (int x = 0; x < output.width(); ++x) {
            const auto movement = adaptive ? motion[qsizetype(row / MotionTileHeight) * tilesAcross + x / MotionTileWidth] : 1.f;
            if (movement <= 0) continue;
            const auto interpolated = bob(current, x, row, parity);
            destination[x] = quantize(movement < 1 ? mix(destination[x], interpolated, movement) : interpolated, frame.pixelFormat);
        }
    }
    if (mode == Deinterlacing::Blend) blendVertical(output, frame.pixelFormat);
    return output;
}

ImageAdjustments normalized(ImageAdjustments value) {
    for (auto* item : {&value.brightness, &value.contrast, &value.saturation, &value.sharpness})
        *item = std::clamp(*item, ImageAdjustments::Minimum, ImageAdjustments::Maximum);
    if (std::find(ImageAdjustments::GammaCurves.begin(), ImageAdjustments::GammaCurves.end(), value.gamma) ==
        ImageAdjustments::GammaCurves.end()) value.gamma = 0;
    return value;
}

AdjustmentParameters adjustmentParameters(ImageAdjustments settings) {
    settings = normalized(settings);
    const float brightness = progress(settings.brightness);
    const float remainingLight = 1.f + brightness;
    const auto amount = brightness < 0 ? (remainingLight * remainingLight - 1.f) * .9998f
        : brightness * brightness * brightness * .935f;
    const float contrast = progress(settings.contrast);
    const auto gain = std::exp2(contrast * (contrast < 0 ? 6.f : 2.125f));
    const float saturation = progress(settings.saturation);
    const auto saturationGain = 1.f + (saturation > 0 ? 2.f * saturation : saturation);
    const float sharpness = progress(settings.sharpness);
    const float gamma = settings.gamma == ImageAdjustments::Bt1886 ? -1.f : float(settings.gamma) / 100.f;
    return {amount, gain, gamma, saturationGain, sharpness > 0 ? 2.f * sharpness : sharpness};
}

ImageRestoration restorationForDisplay(const Profile& profile) {
    auto restoration = normalized(profile.restoration);
    const auto* signal = profile.displayTechnology() == DisplayTechnology::Crt ? &profile.crt : profile.panelSignal();
    if (!profile.supportsDedithering() || (signal && signal->mergesDithering())) restoration.dedithering = 0;
    if (!profile.usesHostDisplay()) {
        restoration.debanding = 0;
        restoration.detailRecovery = 0;
    }
    return restoration;
}

bool requiresProcessing(const Profile& profile, SignalSimulation signal, QSize inputSize, QSize outputSize) {
    const auto restoration = restorationForDisplay(profile);
    const bool displayFilter = !outputSize.isEmpty() &&
        (profile.displayTechnology() != DisplayTechnology::Normal || usesSoftwareSampling(profile.displaySampling()) ||
            (restoration.detailRecovery > 0 && profile.displaySampling() != Sampling::Nearest && outputSize != inputSize));
    const bool usesOwnSignal = profile.displayTechnology() == DisplayTechnology::Crt || profile.panelSignal();
    return displayFilter || (!usesOwnSignal && hasSignalEffect(normalized(signal))) || profile.effectiveImageAdjustments() != ImageAdjustments{} ||
        restoration.dedithering || restoration.debanding;
}

QImage process(const QImage& input, const Profile& profile, SignalSimulation signal,
        const Emulation::VideoFrame& frame, QSize outputSize, DisplayHistory& history, bool advance,
        std::optional<Emulation::VideoSegmentLevels> segmentLevels, Emulation::VideoVectorSegments vectors) {
    const auto technology = outputSize.isEmpty() ? DisplayTechnology::Normal : profile.displayTechnology();
    const bool crt = technology == DisplayTechnology::Crt;
    const bool dlp = technology == DisplayTechnology::Projection &&
        profile.projection.technology == ProjectionTechnology::SingleChipDlp;
    if ((!dlp || input.isNull()) && !history.projection.previous.isNull()) history.projection = {};
    const bool vectorDisplay = technology == DisplayTechnology::Vector;
    const bool nativeVectors = vectorDisplay && bool(vectors);
    if (!vectorDisplay) history.vector = {};
    if ((input.isNull() && !nativeVectors) || !requiresProcessing(profile, signal, input.size(), outputSize)) return input;
    const auto settings = profile.effectiveImageAdjustments();
    const bool fixedPixel = technology == DisplayTechnology::FixedPixel;
    const bool plasma = technology == DisplayTechnology::Plasma;
    const bool oled = fixedPixel && profile.fixedPixel.technology == FixedPixelTechnology::Oled;
    const bool ledBacklit = fixedPixel && profile.fixedPixel.technology == FixedPixelTechnology::LedBacklitLcd;
    const bool vfd = technology == DisplayTechnology::Vfd;
    const bool dotMatrix = technology == DisplayTechnology::DotMatrix;
    const auto crtProfile = normalized(profile.crt);
    const LcdProfile& lcd = ledBacklit ? static_cast<const LcdProfile&>(profile.ledBacklight) : profile.lcd;
    const auto vectorProfile = normalized(profile.vector);
    const auto segments = normalized(profile.segmentDisplay);
    const auto& ePaper = profile.ePaper;
    const bool electronicPaper = technology == DisplayTechnology::EPaper;
    const bool segmentDisplay = technology == DisplayTechnology::SegmentDisplay;
    const auto restoration = restorationForDisplay(profile);
    const bool recover = restoration.detailRecovery > 0 && profile.displaySampling() != Sampling::Nearest &&
        !outputSize.isEmpty() && outputSize != input.size();
    const bool softwareSampling = usesSoftwareSampling(profile.displaySampling()) && !outputSize.isEmpty();
    const bool displayFilter = technology != DisplayTechnology::Normal || softwareSampling || recover;
    signal = normalized(signal);
    const auto* panel = profile.panelSignal();
    const bool analog = profile.displayTechnology() != DisplayTechnology::Crt && !panel && hasSignalEffect(signal);
    constexpr float MiddleGray = .21586f;
    const auto parameters = adjustmentParameters(settings);
    const auto amount = parameters.brightness;
    const auto gain = parameters.contrast;
    const auto saturationGain = parameters.saturation;
    const auto sharpness = parameters.sharpness;
    const auto sharpnessAmount = std::abs(sharpness);
    static const float blackRoot = std::pow(.001f, 1.f / 2.4f);
    static const auto linearValues = [] {
        std::array<float, ColorMaximum + 1> values{};
        for (size_t i = 0; i < values.size(); ++i) values[i] = linear(float(i) / ColorMaximum);
        return values;
    }();
    const auto size = displayFilter ? outputSize : input.size();
    const int width = size.width(), height = size.height();
    const auto decode = [](QRgb pixel) -> Color {
        return {linearValues[qRed(pixel)], linearValues[qGreen(pixel)], linearValues[qBlue(pixel)]};
    };
    const auto transform = [&](Color color) {
        for (auto& channel : color) {
            if (settings.contrast) channel = std::clamp(MiddleGray + (channel - MiddleGray) * gain, 0.f, 1.f);
            if (settings.brightness && !crt && !vectorDisplay) channel = std::lerp(channel, amount < 0 ? 0.f : 1.f, std::abs(amount));
        }
        if (settings.gamma) {
            const float light = luminance(color);
            const float signal = srgb(light);
            const float corrected = settings.gamma == ImageAdjustments::Bt1886
                ? std::pow(std::lerp(blackRoot, 1.f, signal), 2.4f) : std::pow(signal, float(settings.gamma) / 100.f);
            if (corrected != light) {
                const float target = corrected > light ? 1.f : 0.f;
                for (auto& channel : color) channel = std::lerp(channel, target, (corrected - light) / (target - light));
            }
        }
        if (settings.saturation) {
            const auto light = luminance(color);
            for (auto& channel : color) channel = std::clamp(light + (channel - light) * saturationGain, 0.f, 1.f);
        }
        return color;
    };
    QImage output(width, height, nativeVectors ? QImage::Format_RGB32 : input.format());
    if (output.isNull()) return output;
    std::vector<Color> colors;
    std::vector<quint8> backlightZones;
    float oledLight = 0.f, plasmaLight = 0.f;
    const auto position = [width](int x, int y) { return size_t(y) * size_t(width) + size_t(x); };
    if (nativeVectors) {
        colors.resize(size_t(width) * height);
    } else if (displayFilter || analog || settings.sharpness || restoration.dedithering || restoration.debanding) {
        const auto prepared = vectorDisplay ? input : dedither(input, restoration.dedithering);
        auto source = prepared;
        if (crt) source = filterCrtConnection(prepared, crtProfile, frame);
        else if (panel) source = filterPanelSignal(prepared, *panel, frame);
        if (dlp) source = separateProjectionColors(source, frame, profile.projection, profile.projectionSignal, history.projection);
        if (oled) oledLight = oledFrameLight(source);
        if (plasma) plasmaLight = plasmaFrameLight(source);
        if (ledBacklit) backlightZones = ledBacklightZones(source, profile.ledBacklight.type);
        const int inputWidth = source.width(), inputHeight = source.height();
        colors.resize(size_t(inputWidth) * size_t(inputHeight));
        for (int y = 0; y < inputHeight; ++y)
            for (int x = 0; x < inputWidth; ++x)
                colors[size_t(y) * size_t(inputWidth) + size_t(x)] = decode(source.pixel(x, y));
        const auto reference = recover ? colors : std::vector<Color>{};
        deband(colors, source.size(), restoration.debanding);
        if (analog && !vectorDisplay) simulateSignal(colors, inputWidth, inputHeight, frame.sequence, signal);
        const auto panelSize = electronicPaper ? DisplaySignalProfile{.definition = ePaper.definition}.panelSize(source.size()) :
            panel ? panel->panelSize(source.size()) : source.size();
        const bool ledMatrix = technology == DisplayTechnology::LedMatrix;
        if (ledMatrix || vfd || dotMatrix) {
            if (settings != ImageAdjustments{}) {
                for (auto& color : colors) color = transform(color);
            }
            if (vfd) applyVfd(colors, size, source.size(), profile.vfd);
            else if (dotMatrix) applyDotMatrix(colors, size, source.size(), profile.dotMatrix);
            else applyLedMatrix(colors, size, source.size(), profile.ledMatrix);
        } else if ((panel || electronicPaper) && (panelSize != source.size() || panelSize != size)) {
            const auto project = [](int position, int from, int to) {
                return int((2 * qint64(position) + 1) * to / (2 * qint64(from)));
            };
            std::vector<int> columns(width);
            for (int x = 0; x < width; ++x)
                columns[x] = project(project(x, width, panelSize.width()), panelSize.width(), inputWidth);
            std::vector<Color> resampled(size_t(width) * size_t(height));
            for (int y = 0; y < height; ++y) {
                const int sourceRow = project(project(y, height, panelSize.height()), panelSize.height(), inputHeight);
                for (int x = 0; x < width; ++x)
                    resampled[position(x, y)] = colors[size_t(sourceRow) * size_t(inputWidth) + size_t(columns[x])];
            }
            colors = std::move(resampled);
        } else if (size != input.size()) {
            std::vector<Color> resampled(size_t(width) * size_t(height));
            const auto sourceIndex = [&](int x, int y) {
                return size_t(std::clamp(y, 0, inputHeight - 1)) * size_t(inputWidth) +
                    size_t(std::clamp(x, 0, inputWidth - 1));
            };
            const auto sample = [&](int x, int y) -> const Color& { return colors[sourceIndex(x, y)]; };
            const bool perceptualSampling = usesPerceptualSampling(profile.displaySampling());
            std::vector<Color> encoded;
            if (perceptualSampling) {
                encoded.resize(colors.size());
                std::ranges::transform(colors, encoded.begin(), [](const Color& color) {
                    return Color{srgb(color[0]), srgb(color[1]), srgb(color[2])};
                });
            }
            const auto sampleEncoded = [&](int x, int y) -> const Color& { return encoded[sourceIndex(x, y)]; };
            const float scaleX = float(width) / float(inputWidth);
            const float scaleY = float(height) / float(inputHeight);
            for (int y = 0; y < height; ++y) {
                const auto row = (float(y) + .5f) / scaleY - .5f;
                for (int x = 0; x < width; ++x) {
                    const auto column = (float(x) + .5f) / scaleX - .5f;
                    auto& target = resampled[position(x, y)];
                    if (vectorDisplay) {
                        target = filterSample(Sampling::Linear, sample, column, row, scaleX, scaleY);
                    } else if (profile.displaySampling() == Sampling::Nearest) {
                        target = sample(int(std::floor(column + .5f)), int(std::floor(row + .5f)));
                    } else if (perceptualSampling) {
                        target = filterPerceptualSample(profile.displaySampling(), sample, sampleEncoded, column, row);
                    } else {
                        target = filterSample(profile.displaySampling(), sample, column, row, scaleX, scaleY);
                    }
                }
            }
            colors = std::move(resampled);
        }
        if (recover) recoverDetail(reference, source.size(), colors, size, restoration.detailRecovery);
        if (!ledMatrix && !vfd && !dotMatrix && !vectorDisplay && settings != ImageAdjustments{}) {
            for (auto& color : colors) color = transform(color);
        }
    }
    if (vectorDisplay) {
        renderVector(colors, size, vectorProfile, settings.brightness, frame, vectors, history.vector);
        if (settings != ImageAdjustments{})
            for (auto& color : colors) color = transform(color);
    }
    const auto component = [](float value) { return int(std::nearbyint(srgb(value) * ColorMaximum)); };
    if (!displayFilter || settings.sharpness) {
        std::vector<Color> sharpened;
        if (displayFilter && settings.sharpness) sharpened.resize(colors.size());
        for (int y = 0; y < height; ++y) {
            auto* row = reinterpret_cast<QRgb*>(output.scanLine(y));
            for (int x = 0; x < width; ++x) {
                auto color = colors.empty() ? transform(decode(input.pixel(x, y))) : colors[position(x, y)];
                if (settings.sharpness) {
                    const auto sample = [&](int dx, int dy) -> const Color& {
                        return colors[position(std::clamp(x + dx, 0, width - 1), std::clamp(y + dy, 0, height - 1))];
                    };
                    Color sum{};
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx)
                            for (size_t i = 0; i < sum.size(); ++i) sum[i] += sample(dx, dy)[i];
                    for (size_t i = 0; i < color.size(); ++i) {
                        const auto average = sharpness < 0
                            ? (sum[i] + .5f * (sample(-2, 0)[i] + sample(2, 0)[i] + sample(0, -2)[i] + sample(0, 2)[i])) / 11.f
                            : sum[i] / 9.f;
                        color[i] += (sharpness > 0 ? color[i] - average : average - color[i]) * sharpnessAmount;
                    }
                }
                if (displayFilter) {
                    if (!sharpened.empty()) sharpened[position(x, y)] = color;
                } else row[x] = qRgb(component(color[0]), component(color[1]), component(color[2]));
            }
        }
        if (!sharpened.empty()) colors = std::move(sharpened);
    }
    if (displayFilter) {
        std::optional<CrtSignal> crtSignal;
        float beamGain = 1.f;
        if (crt) {
            const auto aspect = float(screenAspectRatio(profile, frameAspectRatio(frame, input.size())));
            crtSignal.emplace(crtProfile, frame, aspect, crtProfile.beamWidth, crtProfile.curvature);
            beamGain = std::exp2(float(settings.brightness) / 10.f);
            applyCrt(colors, size, input.size(), crtProfile, *crtSignal, beamGain);
        }
        if (oled) {
            applyOledLight(colors, profile.oled.panelType, oledLight);
            applyOledStructure(colors, size, profile.oledSignal.panelSize(input.size()), profile.oled.pixelStructure);
        } else if (fixedPixel) {
            applyLcdStructure(colors, size, panel->panelSize(input.size()), lcd.pixelStructure, lcd.orientation);
            if (!ledBacklit) applyLcdLight(colors, size, lcd);
        }
        if (plasma)
            applyPlasma(colors, size, panel->panelSize(input.size()), frame.sequence, profile.plasma.cellStructure, plasmaLight);
        const bool nativeSegments = segmentDisplay && segmentLevels && applyNativeSegments(colors, size, segments, frame, *segmentLevels);
        if (segmentDisplay && !nativeSegments) applySegmentDisplay(colors, size, input.size(), segments);
        if (electronicPaper) applyEPaper(colors, ePaper.colorMode);
        if (technology == DisplayTechnology::Projection)
            applyProjection(colors, size, input.size(), profile.projectionSignal.panelSize(input.size()), normalized(profile.projection));
        if (ledBacklit) applyLedBacklight(colors, size, input.size(), backlightZones, profile.ledBacklight);
        if (fixedPixel && !oled) applyLcdResponse(colors, size, lcd.panelType, frame.timestamp, history.fixedPixel, advance);
        if (plasma) applyPlasmaPersistence(colors, size, frame.sequence, history.plasma, advance);
        if (nativeSegments || !segmentDisplay) history.segmentDisplay = {};
        else applySegmentTemporal(colors, size, frame.timestamp, segments, history.segmentDisplay, advance);
        if (electronicPaper) applyEPaperTemporal(colors, size, frame.timestamp, ePaper, history.ePaper);
        else history.ePaper = {};
        if (crtSignal) applyCrtMask(colors, size, crtProfile, *crtSignal, beamGain);
        for (int y = 0; y < height; ++y) {
            auto* row = reinterpret_cast<QRgb*>(output.scanLine(y));
            for (int x = 0; x < width; ++x) {
                const auto& color = colors[position(x, y)];
                row[x] = qRgb(component(color[0]), component(color[1]), component(color[2]));
            }
        }
    }
    return output;
}
}
