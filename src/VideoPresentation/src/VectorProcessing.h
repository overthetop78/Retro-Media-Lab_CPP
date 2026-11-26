#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct VectorHistory {
    QSize size;
    quint64 sequence = 0;
    std::chrono::nanoseconds timestamp{};
    VectorProfile profile;
    int brightness = 0;
    bool native = false;
    std::vector<LinearColor> emission;
};

struct VectorOptics {
    LinearColor tint;
    float scale, sigma, diffusion, halfLife;
};
VectorOptics vectorOptics(const VectorProfile&, QSize);
VectorProfile normalized(VectorProfile);
void renderVector(std::vector<LinearColor>&, QSize, const VectorProfile&, int brightness,
    const Emulation::VideoFrame&, const Emulation::VideoVectorSegments&, VectorHistory&);
}
