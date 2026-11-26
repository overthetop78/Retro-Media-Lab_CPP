#pragma once

#include "AnalogSignal.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
struct FixedPixelHistory {
    QSize size;
    std::chrono::nanoseconds timestamp{};
    std::vector<LinearColor> colors;
};

FixedPixelProfile normalized(FixedPixelProfile);
float periodicCoverage(float center, float span, float first, float last);
LinearColor cellAxis(int pixel, int cells, int pixels, bool striped, bool reverse, float gap = 0.f);
void applyLcdStructure(std::vector<LinearColor>&, QSize output, QSize panel, LcdPixelStructure, LcdPixelOrientation);
void applyLcdLight(std::vector<LinearColor>&, QSize, const LcdProfile&);
std::vector<quint8> ledBacklightZones(const QImage& source, LedBacklightType);
void applyLedBacklight(std::vector<LinearColor>&, QSize output, QSize source, const std::vector<quint8>& zones,
    const LedBacklightProfile&);
float oledFrameLight(const QImage&);
void applyOledLight(std::vector<LinearColor>&, OledPanelType, float frameLight);
void applyOledStructure(std::vector<LinearColor>&, QSize output, QSize panel, OledPixelStructure);
void applyLcdResponse(std::vector<LinearColor>&, QSize, LcdPanelType,
    std::chrono::nanoseconds timestamp, FixedPixelHistory&, bool advance);
}
