#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
LedMatrixProfile normalized(LedMatrixProfile);
void applyLedMatrix(std::vector<LinearColor>&, QSize output, QSize source, const LedMatrixProfile&);
}
