#pragma once

#include <RetroMediaLab/VideoPresentation/Presentation.h>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
Emulation::VideoStandard crtSourceStandard(const Emulation::VideoFrame&);
QImage filterCrtConnection(const QImage&, const DisplaySignalProfile&, const Emulation::VideoFrame&);
}
