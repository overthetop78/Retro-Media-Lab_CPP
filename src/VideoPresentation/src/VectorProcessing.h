#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct VectorHistory {
    QSize size;
    quint64 sequence = 0;
    std::vector<LinearColor> colors;
};

VectorProfile normalized(VectorProfile);
void applyVector(std::vector<LinearColor>&, QSize, const VectorProfile&);
void applyVectorPersistence(std::vector<LinearColor>&, QSize, quint64 sequence, int intensity, VectorHistory&, bool advance);
}
