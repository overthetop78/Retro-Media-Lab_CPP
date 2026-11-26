#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct PlasmaHistory {
    QSize size;
    quint64 sequence = 0;
    std::vector<LinearColor> colors;
};

struct PlasmaEmissionSettings {
    static constexpr float Persistence = .12f;
    static constexpr float PhosphorIntensity = .22f;
    static constexpr float BrightnessLimiter = .65f;
    static constexpr float Dithering = .45f;
    static constexpr int DitherPhases = 16;
};

float plasmaFrameLight(const QImage&);
void applyPlasma(std::vector<LinearColor>&, QSize output, QSize panel, quint64 sequence, PlasmaCellStructure, float frameLight);
void applyPlasmaPersistence(std::vector<LinearColor>&, QSize, quint64 sequence, PlasmaHistory&, bool advance);
}
