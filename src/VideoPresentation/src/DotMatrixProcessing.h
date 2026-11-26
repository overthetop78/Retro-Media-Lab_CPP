#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
LinearColor dotMatrixTint(DotMatrixPalette);
void applyDotMatrix(std::vector<LinearColor>&, QSize output, QSize source, const DotMatrixProfile&);
}
