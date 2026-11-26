#pragma once

#include "DotMatrixProcessing.h"
#include "FixedPixelProcessing.h"
#include "PlasmaProcessing.h"
#include "PaperProjectionProcessing.h"
#include "SegmentDisplayProcessing.h"
#include "VectorProcessing.h"
#include "VfdProcessing.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
double frameAspectRatio(const Emulation::VideoFrame&, QSize fallback);
double screenAspectRatio(const Profile&, double imageAspect);

struct DisplayHistory {
    FixedPixelHistory fixedPixel;
    PlasmaHistory plasma;
    VectorHistory vector;
    VfdHistory vfd;
    DotMatrixHistory dotMatrix;
    SegmentHistory segmentDisplay;
    EPaperHistory ePaper;
    ProjectionColorHistory projection;
};
QImage composeFields(const QImage& current, const QImage& previous, const QImage& sameParity,
    const QImage& previousComplement, const Emulation::VideoFrame&, Deinterlacing,
    QList<float>& motion, std::optional<quint64>& motionSequence);
ImageRestoration restorationForDisplay(const Profile&);
bool requiresProcessing(const Profile&, SignalSimulation, QSize inputSize, QSize outputSize);
QImage process(const QImage&, const Profile&, SignalSimulation, const Emulation::VideoFrame&, QSize outputSize, DisplayHistory&, bool advance);
ImageAdjustments normalized(ImageAdjustments);
struct AdjustmentParameters {
    float brightness, contrast, gamma, saturation, sharpness;
};
AdjustmentParameters adjustmentParameters(ImageAdjustments);
}
