#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct SegmentHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
    std::vector<LinearColor> colors;
};

SegmentDisplayProfile normalized(SegmentDisplayProfile);
void applySegmentDisplay(std::vector<LinearColor>&, QSize output, QSize source, const SegmentDisplayProfile&);
void applySegmentTemporal(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds,
    const SegmentDisplayProfile&, SegmentHistory&, bool advance);
}
