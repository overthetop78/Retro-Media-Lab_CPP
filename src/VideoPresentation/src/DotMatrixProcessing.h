#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct DotMatrixHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
    std::vector<LinearColor> colors;
};

DotMatrixProfile normalized(DotMatrixProfile);
void applyDotMatrix(std::vector<LinearColor>&, QSize output, QSize source, const DotMatrixProfile&);
void applyDotMatrixTemporal(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds, const DotMatrixProfile&,
    DotMatrixHistory&, bool advance);
}
