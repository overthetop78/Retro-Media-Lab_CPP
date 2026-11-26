#pragma once

#include "CrtProcessing.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {

struct RasterSegmentParameters {
    static constexpr size_t MaximumElements = 19;
    std::array<float, 4> geometry{}, shape{}, light{}, timing{};
    std::array<std::array<float, 4>, MaximumElements> lines{};
};

struct DisplayShaderParameters {
    CrtShaderParameters crt;
    std::array<float, 4> ledMatrix{}; // Columns, rows, color and shape.
    std::array<float, 4> vfdStructure{}, vfdPhosphor{};
    std::array<float, 4> dotMatrixGeometry{}, dotMatrixOptics{};
    std::array<float, 4> nativeSegmentLevels{}, nativeSegmentOptics{};
    RasterSegmentParameters rasterSegments;
    std::array<float, 4> ePaperInkAndColor{}, ePaperSurface{}, ePaperTemporal{};
    std::array<float, 4> projectionGeometry{}, projectionOptics{}, projectionScreen{};
    std::array<float, 4> plasmaGeometry{}, plasmaEmission{}, plasmaTiming{};
    std::array<float, 4> vectorPhosphor{}, vectorBeam{}, vectorSource{};
    std::array<float, 4> fixedGrid{}, fixedOptics{}, fixedLight{}, fixedTiming{};
    bool enabled() const {
        return crt.enabled() || ledMatrix[0] > 0.f || vfdStructure[0] > 0.f || dotMatrixGeometry[0] > 0.f ||
            nativeSegmentLevels[1] > nativeSegmentLevels[0] || rasterSegments.geometry[0] > 0.f || ePaperInkAndColor[0] > 0.f ||
            projectionGeometry[0] > 0.f || plasmaGeometry[0] > 0.f || vectorSource[0] > 0.f || fixedGrid[0] > 0.f;
    }
};

}
