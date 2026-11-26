#pragma once

#include "AnalogSignal.h"
#include "DisplayShaderParameters.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct SegmentHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
    std::vector<LinearColor> colors;
};

SegmentDisplayProfile normalized(SegmentDisplayProfile);
LinearColor segmentTint(SegmentColor);
RasterSegmentParameters rasterSegmentParameters(const SegmentDisplayProfile&, QSize, double elapsedMilliseconds);
void applySegmentDisplay(std::vector<LinearColor>&, QSize output, QSize source, const SegmentDisplayProfile&);
bool applyNativeSegments(std::vector<LinearColor>&, QSize output, const SegmentDisplayProfile&,
    const Emulation::VideoFrame&, Emulation::VideoSegmentLevels);
void applySegmentTemporal(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds,
    const SegmentDisplayProfile&, SegmentHistory&, bool advance);
}
