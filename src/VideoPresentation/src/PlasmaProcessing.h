#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct PlasmaHistory {
    QSize size;
    quint64 sequence = 0;
    std::vector<LinearColor> colors;
};

float plasmaFrameLight(const QImage&);
void applyPlasma(std::vector<LinearColor>&, QSize output, QSize panel, quint64 sequence, PlasmaCellStructure, float frameLight);
void applyPlasmaPersistence(std::vector<LinearColor>&, QSize, quint64 sequence, PlasmaHistory&, bool advance);
}
