#pragma once

#include <RetroMediaLab/VideoPresentation/Presentation.h>
#include <vector>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
using LinearColor = std::array<float, 3>;
SignalSimulation normalized(SignalSimulation);
bool hasSignalEffect(SignalSimulation);
void simulateSignal(std::vector<LinearColor>&, int width, int height, quint64 sequence, SignalSimulation);
}
