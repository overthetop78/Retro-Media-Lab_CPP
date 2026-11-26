#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct EPaperRefresh {
    bool captureHistory;
    bool hasPrevious;
    float currentWeight;
    float inversion;
};

class EPaperRefreshClock {
public:
    EPaperRefresh advance(const EPaperProfile&, QSize, std::chrono::nanoseconds);

private:
    EPaperProfile profile_;
    QSize size_;
    std::chrono::nanoseconds timestamp_{};
    std::chrono::nanoseconds refreshStarted_{};
    int refreshCount_ = 0;
};

struct EPaperHistory {
    EPaperRefreshClock clock;
    std::vector<LinearColor> colors;
};

struct ProjectionColorHistory {
    ProjectionProfile configuration;
    DisplaySignalProfile signal;
    quint64 sequence = 0;
    std::chrono::nanoseconds timestamp{};
    QImage previous;
    QImage separated;
};

QImage separateProjectionColors(const QImage&, const Emulation::VideoFrame&,
    const ProjectionProfile&, const DisplaySignalProfile&, ProjectionColorHistory&);
std::array<float, 4> ePaperSurface(EPaperColorMode);
ProjectionProfile normalized(ProjectionProfile);
struct ProjectionOptics {
    float black, blur, border, convergence;
    float reflection, ambient, texture;
};
ProjectionOptics projectionOptics(const ProjectionProfile&);
void applyEPaper(std::vector<LinearColor>&, EPaperColorMode);
void applyEPaperTemporal(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds,
    const EPaperProfile&, EPaperHistory&);
void applyProjection(std::vector<LinearColor>&, QSize output, QSize source, QSize panel, const ProjectionProfile&);
}
