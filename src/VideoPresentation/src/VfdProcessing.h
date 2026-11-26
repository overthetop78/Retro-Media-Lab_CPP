#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
LinearColor vfdPhosphor(VfdColor);
void applyVfd(std::vector<LinearColor>&, QSize output, QSize source, const VfdProfile&);
}
