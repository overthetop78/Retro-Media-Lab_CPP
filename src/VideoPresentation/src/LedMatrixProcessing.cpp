#include "LedMatrixProcessing.h"
#include "MatrixCells.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
namespace {
constexpr std::array<LinearColor, 6> Tints{{
    {1.f, 1.f, 1.f}, {1.f, .03f, .01f}, {.03f, 1.f, .08f}, {1.f, .42f, .02f}, {.03f, .25f, 1.f}, {1.f, 1.f, 1.f}
}};

float unit(float value) { return std::clamp(value, 0.f, 1.f); }

float cellDistance(float x, float y, LedMatrixShape shape) {
    constexpr float RoundRadius = .36f, SquareRadius = .34f;
    if (shape == LedMatrixShape::Round) return std::sqrt(x * x + y * y) - RoundRadius;
    constexpr float RoundedCornerRadius = .10f;
    const float corner = shape == LedMatrixShape::RoundedSquare ? RoundedCornerRadius : 0.f;
    const float qx = std::abs(x) - (SquareRadius - corner);
    const float qy = std::abs(y) - (SquareRadius - corner);
    if (!corner) return std::max(qx, qy);
    const float outerX = std::max(qx, 0.f), outerY = std::max(qy, 0.f);
    return std::sqrt(outerX * outerX + outerY * outerY) + std::min(std::max(qx, qy), 0.f) - corner;
}

}

void applyLedMatrix(std::vector<LinearColor>& colors, QSize output, QSize source, const LedMatrixProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const auto grid = matrixSize(profile.definition, source);
    const int columns = grid.width(), rows = grid.height();
    std::vector<LinearColor> emissions(size_t(columns) * rows);
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < columns; ++x) {
            auto cell = sampleMatrixCell(colors, source, grid, x, y);
            for (auto& value : cell) value = unit(value);
            if (profile.color == LedMatrixColor::RedGreen) {
                constexpr float RedGreenGreenGain = .5f;
                cell = {cell[0], cell[1] * RedGreenGreenGain, 0.f};
            } else if (profile.color != LedMatrixColor::Rgb) {
                const auto& tint = Tints[size_t(profile.color)];
                const float luminance = matrixLuminance(cell);
                for (size_t c = 0; c < cell.size(); ++c) cell[c] = luminance * tint[c];
            }
            emissions[size_t(y) * columns + x] = cell;
        }
    colors.resize(size_t(output.width()) * output.height());
    const float stepX = float(columns) / output.width(), stepY = float(rows) / output.height();
    constexpr float MinimumEdge = .015f, MaximumEdge = .20f;
    constexpr float DetailStart = 1.f, DetailEnd = 2.5f;
    const float cellWidth = float(output.width()) / columns, cellHeight = float(output.height()) / rows;
    const float cellScale = std::min(cellWidth, cellHeight);
    const float scaleX = cellWidth / cellScale, scaleY = cellHeight / cellScale;
    const float edge = std::clamp(.5f / cellScale, MinimumEdge, MaximumEdge);
    const float detail = matrixSmoothStep(DetailStart, DetailEnd, cellScale);
    for (int y = 0; y < output.height(); ++y) {
        const auto matrixY = (y + .5f) * stepY;
        const int row = std::min(rows - 1, int(matrixY));
        const float dy = matrixY - row - .5f;
        for (int x = 0; x < output.width(); ++x) {
            const auto matrixX = (x + .5f) * stepX;
            const int column = std::min(columns - 1, int(matrixX));
            const float dx = matrixX - column - .5f;
            const float distance = cellDistance(dx * scaleX, dy * scaleY, profile.shape);
            const float core = 1.f - matrixSmoothStep(-edge, edge, distance);
            const auto& emission = emissions[size_t(row) * columns + column];
            const float energy = *std::max_element(emission.begin(), emission.end());
            constexpr float HaloBaseRadius = .05f, HaloEnergyRadius = .09f, HaloGain = .25f;
            const float halo = (1.f - matrixSmoothStep(0.f, HaloBaseRadius + HaloEnergyRadius * energy, std::max(distance, 0.f))) *
                (1.f - core) * (HaloGain * energy);
            constexpr float PanelBlack = .0045f, UnlitCell = .010f;
            auto& color = colors[size_t(y) * output.width() + x];
            for (size_t channel = 0; channel < color.size(); ++channel) {
                const float cell = unit(PanelBlack + UnlitCell * core + emission[channel] * (core + halo));
                color[channel] = std::lerp(emission[channel], cell, detail);
            }
        }
    }
}
}
