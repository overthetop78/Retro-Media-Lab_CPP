#pragma once

#include "SoftwareProcessing.h"
#include "DisplayShaderParameters.h"
#include "RestorationProcessing.h"
#include <QFile>
#include <rhi/qshader.h>

namespace RetroMediaLab::VideoPresentation::HardwareSamplingShader {

inline constexpr auto VertexShader = ":/video-presentation/sampling.vert.qsb";
inline constexpr auto FragmentShader = ":/video-presentation/sampling.frag.qsb";
inline constexpr auto CrtFragmentShader = ":/video-presentation/crt.frag.qsb";
inline constexpr int ParametersBinding = 0;
inline constexpr int PointSamplerBinding = 1;
inline constexpr int LinearSamplerBinding = 2;

struct Parameters {
    float output[4];
    qint32 settings[4];
    float adjustments[4];
    float corrections[4];
    SoftwareProcessing::DisplayShaderParameters crt;
};
static_assert(sizeof(Parameters) == 864);

inline Parameters makeParameters(QSize size, Sampling sampling, int ySign, bool overlay, ImageAdjustments settings,
        ImageRestoration restoration, const SoftwareProcessing::DisplayShaderParameters& crt = {}) {
    if (crt.crt.enabled() || crt.vectorSource[0] > 0.f) settings.brightness = 0;
    const auto image = SoftwareProcessing::adjustmentParameters(settings);
    restoration = SoftwareProcessing::normalized(restoration);
    const auto maximum = float(ImageRestoration::MaximumIntensity);
    return {{float(size.width()), float(size.height()), 0.f, 0.f},
        {qint32(sampling), ySign, overlay ? 1 : 0, 0},
        {image.brightness, image.contrast, image.gamma, image.saturation}, {image.sharpness, restoration.debanding / maximum, restoration.detailRecovery / maximum, 0.f}, crt};
}

inline QShader readShader(const char* path) {
    QFile file(QString::fromLatin1(path));
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QShader::fromSerialized(file.readAll());
}

} // namespace RetroMediaLab::VideoPresentation::HardwareSamplingShader
