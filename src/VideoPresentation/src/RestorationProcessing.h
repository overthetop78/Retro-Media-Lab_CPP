#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
ImageRestoration normalized(ImageRestoration);
QImage dedither(const QImage&, int intensity);
void deband(std::vector<LinearColor>&, QSize, int intensity);
void recoverDetail(const std::vector<LinearColor>&, QSize source, std::vector<LinearColor>&, QSize output, int intensity);
}
