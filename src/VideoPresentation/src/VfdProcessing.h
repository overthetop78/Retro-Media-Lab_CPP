#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct VfdHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
    std::vector<LinearColor> colors;
};

VfdProfile normalized(VfdProfile);
void applyVfd(std::vector<LinearColor>&, QSize output, QSize source, const VfdProfile&);
void applyVfdPersistence(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds timestamp, int milliseconds,
    VfdHistory&, bool advance);
}
