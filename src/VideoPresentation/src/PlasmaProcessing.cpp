#include "PlasmaProcessing.h"
#include "FixedPixelProcessing.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr std::array Bayer{
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5
};
constexpr quint64 MaximumElapsedFrames = 120;
constexpr float BlackFloor = .0025f;
constexpr float WaffleGap = .018f;
constexpr int LightSampleCount = 4096;
constexpr LinearColor Amber{1.f, .29f, .025f};

float unit(float value) { return std::clamp(value, 0.f, 1.f); }
float fraction(float value) { return value - std::floor(value); }
float luma(const LinearColor& c) { return c[0] * .2126f + c[1] * .7152f + c[2] * .0722f; }

float smoothStep(float low, float high, float value) {
    const auto t = unit((value - low) / (high - low));
    return t * t * (3.f - 2.f * t);
}

void applyCells(std::vector<LinearColor>& colors, QSize output, QSize panel, PlasmaCellStructure structure) {
    using enum PlasmaCellStructure;
    const int width = output.width(), height = output.height();
    if (width <= panel.width() && height <= panel.height() && structure != Monochrome) return;
    const float spanX = float(panel.width()) / width, spanY = float(panel.height()) / height;
    const float delta = structure == Delta ? smoothStep(2.f, 5.f, 1.f / std::max(spanX, spanY)) : 0.f;
    const bool striped = structure == Stripe || structure == Waffle;
    std::vector<LinearColor> columns(width);
    std::vector<float> neutralColumns(width);
    for (int x = 0; x < width; ++x) {
        const float neutral = cellAxis(x, panel.width(), width, false, false)[0];
        neutralColumns[x] = neutral;
        if (striped) {
            columns[x] = cellAxis(x, panel.width(), width, true, false, structure == Waffle ? WaffleGap : 0.f);
        } else if (delta > 0.f) {
            const float phase = fraction((x + .5f) * spanX);
            columns[x] = {periodicCoverage(phase, spanX, .04f, .46f), periodicCoverage(phase, spanX, .29f, .71f),
                periodicCoverage(phase, spanX, .54f, .96f)};
        } else {
            columns[x] = {neutral, neutral, neutral};
        }
    }
    for (int y = 0; y < height; ++y) {
        const float neutralRow = cellAxis(y, panel.height(), height, false, false)[0];
        const float phase = fraction((y + .5f) * spanY);
        const float lower = delta > 0.f ? periodicCoverage(phase, spanY, .54f, .96f) : 0.f;
        const float upper = delta > 0.f ? periodicCoverage(phase, spanY, .04f, .46f) : 0.f;
        const LinearColor deltaRows{lower, upper, lower};
        for (int x = 0; x < width; ++x) {
            auto& color = colors[size_t(y) * width + x];
            const float luminance = structure == Monochrome ? luma(color) : 0.f;
            for (size_t channel = 0; channel < color.size(); ++channel) {
                float aperture = columns[x][channel];
                if (delta > 0.f)
                    aperture = std::lerp(neutralColumns[x] * neutralRow, aperture * deltaRows[channel], delta);
                else if (structure != Stripe)
                    aperture *= neutralRow;
                if (structure == Monochrome) color[channel] = luminance * Amber[channel];
                color[channel] *= aperture;
            }
        }
    }
}
}

float plasmaFrameLight(const QImage& source) {
    if (source.isNull()) return 0.f;
    const auto pixelCount = qint64(source.width()) * source.height();
    const auto stride = std::max<qint64>(1, pixelCount / LightSampleCount);
    float sum = 0.f;
    int samples = 0;
    for (qint64 i = 0; i < pixelCount; i += stride) {
        const auto rgb = source.pixel(int(i % source.width()), int(i / source.width()));
        const LinearColor channels{qRed(rgb) / 255.f, qGreen(rgb) / 255.f, qBlue(rgb) / 255.f};
        sum += luma({channels[0] * channels[0], channels[1] * channels[1], channels[2] * channels[2]});
        ++samples;
    }
    return sum / samples;
}

void applyPlasma(std::vector<LinearColor>& colors, QSize output, QSize panel, quint64 sequence, PlasmaCellStructure structure, float frameLight) {
    if (colors.empty() || output.isEmpty() || panel.isEmpty()) return;
    const float gain = 1.f - PlasmaEmissionSettings::BrightnessLimiter * .42f * smoothStep(.28f, .78f, frameLight);
    const float levels = std::lerp(255.f, 31.f, PlasmaEmissionSettings::Dithering);
    const int phase = int(sequence % PlasmaEmissionSettings::DitherPhases);
    for (int y = 0; y < output.height(); ++y) {
        const int cellY = int((2 * qint64(y) + 1) * panel.height() / (2 * qint64(output.height())));
        for (int x = 0; x < output.width(); ++x) {
            auto& color = colors[size_t(y) * output.width() + x];
            const float luminance = luma(color);
            const int cellX = int((2 * qint64(x) + 1) * panel.width() / (2 * qint64(output.width())));
            const float threshold = (Bayer[((cellY + phase / 4) & 3) * 4 + ((cellX + phase) & 3)] + .5f) / 16.f;
            for (auto& value : color) {
                const float saturated = luminance + (value - luminance) * (1.f + PlasmaEmissionSettings::PhosphorIntensity * .28f);
                const float original = unit(saturated + std::max(0.f, saturated - .42f) * PlasmaEmissionSettings::PhosphorIntensity * .48f) * gain;
                const float quantized = std::floor(original * levels + threshold) / levels;
                value = std::lerp(BlackFloor, 1.f, unit(std::lerp(original, quantized, PlasmaEmissionSettings::Dithering)));
            }
        }
    }
    applyCells(colors, output, panel, structure);
}

void applyPlasmaPersistence(std::vector<LinearColor>& colors, QSize size, quint64 sequence, PlasmaHistory& history, bool advance) {
    if (history.size == size && history.colors.size() == colors.size() && sequence >= history.sequence) {
        if (!advance && sequence == history.sequence) {
            colors = history.colors;
            return;
        }
        const auto elapsed = std::clamp<quint64>(sequence - history.sequence, 1, MaximumElapsedFrames);
        const float decay = std::pow(PlasmaEmissionSettings::Persistence, float(elapsed));
        for (size_t i = 0; i < colors.size(); ++i)
            for (size_t c = 0; c < colors[i].size(); ++c)
                colors[i][c] = unit(std::max(colors[i][c], history.colors[i][c] * decay));
    }
    if (advance) history = {size, sequence, colors};
}
}
