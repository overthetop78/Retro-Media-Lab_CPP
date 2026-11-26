#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
StylisticEffects normalized(StylisticEffects);
void applyStylistic(std::vector<LinearColor>&, QSize, quint64 sequence, const StylisticEffects&);
}
