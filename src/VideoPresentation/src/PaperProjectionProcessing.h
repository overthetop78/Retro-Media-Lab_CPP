#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct EPaperHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
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
EPaperProfile normalized(EPaperProfile);
ProjectionProfile normalized(ProjectionProfile);
void applyEPaper(std::vector<LinearColor>&, QSize, const EPaperProfile&);
void applyEPaperTemporal(std::vector<LinearColor>&, QSize, std::chrono::nanoseconds,
    const EPaperProfile&, EPaperHistory&, bool advance);
void applyProjection(std::vector<LinearColor>&, QSize output, QSize source, QSize panel, const ProjectionProfile&);
}
