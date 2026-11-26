#pragma once

#include "AnalogSignal.h"
#include <QSize>
#include <QVector2D>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
enum class CrtDecoder { Monochrome, Ntsc, Pal, Secam, Separate };

struct CrtSignal {
    CrtSignal(const DisplaySignalProfile&, const Emulation::VideoFrame&, float aspect,
        int beamWidth = 0, int screenCurvature = 0);

    QVector2D screen(QVector2D uv) const;
    QVector2D footprint(QVector2D uv, QVector2D pixelSize) const;
    QVector2D source(QVector2D uv) const;
    float beam(float y, int height, CrtMask mask) const;

    float verticalScale, halfLines, aperture, phase, ratio, lineSlip, curvature, exponent;
    QVector2D grid;
    CrtDecoder decoder;
};

struct CrtShaderParameters {
    std::array<float, 4> raster{}, tuning{}, phosphor{}, geometry{};
    bool enabled() const { return geometry[2] > 0.f; }
};

CrtShaderParameters crtShaderParameters(const CrtProfile&, const Emulation::VideoFrame&, float aspect, int brightness);
CrtProfile normalized(CrtProfile);
QImage filterPanelSignal(const QImage&, const DisplaySignalProfile&, const Emulation::VideoFrame&);
void applyCrt(std::vector<LinearColor>&, QSize output, QSize source, const CrtProfile&,
    const CrtSignal&, float gain);
void applyCrtMask(std::vector<LinearColor>&, QSize, const CrtProfile&, const CrtSignal&, float gain);
}
