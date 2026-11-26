#include "DotMatrixProcessing.h"
#include "MatrixCells.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {

LinearColor dotMatrixTint(DotMatrixPalette palette) {
    switch (palette) {
    case DotMatrixPalette::Gray: return {.55f, .55f, .55f};
    case DotMatrixPalette::Green: return {.38f, .71f, .025f};
    case DotMatrixPalette::Amber: return {1.f, .45f, .032f};
    case DotMatrixPalette::Red: return {1.f, .025f, .015f};
    case DotMatrixPalette::Blue: return {.035f, .22f, 1.f};
    default: return {};
    }
}

void applyDotMatrix(std::vector<LinearColor>& colors, QSize output, QSize source, const DotMatrixProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const auto grid = matrixSize(profile.definition, source);
    const int columns = grid.width(), rows = grid.height();
    const bool round = profile.shape == DotMatrixShape::Round;
    const bool rectangle = profile.shape == DotMatrixShape::Rectangle;
    const bool rgb = profile.palette == DotMatrixPalette::Rgb;
    const bool emissive = profile.technology == DotMatrixTechnology::Emissive;
    const bool backlit = profile.technology == DotMatrixTechnology::Backlit;
    const auto palette = dotMatrixTint(profile.palette);
    constexpr LinearColor BacklitBackground{.70f, .72f, .69f};
    constexpr LinearColor ReflectiveBackground{.462f, .533f, .258f};
    const auto& background = backlit ? BacklitBackground : ReflectiveBackground;
    constexpr float MinimumLevel = .0001f;
    constexpr float EmissiveBlack = .004f, EmissiveBase = .009f, EmissiveGain = .97f, ActivatedBase = .014f;
    const float transmission = backlit ? .085f : .042f;
    const float chromaticTransmission = backlit ? .60f : .42f;
    const float inactiveGain = backlit ? .94f : .96f;
    struct Cell { LinearColor active; float level; };
    std::vector<Cell> cells(size_t(columns) * rows);
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < columns; ++x) {
            const auto sample = sampleMatrixCell(colors, source, grid, x, y);
            const auto [low, high] = std::minmax_element(sample.begin(), sample.end());
            auto& cell = cells[size_t(y) * columns + x];
            cell.level = rgb ? *high : matrixLuminance(sample);
            const float denominator = std::max(cell.level, MinimumLevel);
            const float chroma = rgb ? (cell.level - *low) / denominator : 0.f;
            const float cellTransmission = std::lerp(transmission, chromaticTransmission, chroma);
            for (size_t channel = 0; channel < cell.active.size(); ++channel) {
                const float tint = rgb ? sample[channel] / denominator : palette[channel];
                cell.active[channel] = emissive ? EmissiveBase + EmissiveGain * tint * cell.level
                    : ActivatedBase + tint * cellTransmission;
            }
        }

    constexpr float Radius = .40f, RectangleHalfWidth = .30f;
    constexpr float RoundArea = .503f, SquareArea = .64f, RectangleArea = .48f;
    constexpr float MaximumEdge = .20f, DetailStart = 1.f, DetailEnd = 2.5f;
    const float openArea = round ? RoundArea : rectangle ? RectangleArea : SquareArea;
    const float halfWidth = rectangle ? RectangleHalfWidth : Radius;
    const float stepX = float(columns) / output.width(), stepY = float(rows) / output.height();
    const float scale = 1.f / std::max(stepX, stepY);
    const float edge = std::min(.5f / std::max(scale, 1.f), MaximumEdge);
    const float detail = matrixSmoothStep(DetailStart, DetailEnd, scale);
    colors.resize(size_t(output.width()) * output.height());
    for (int y = 0; y < output.height(); ++y) {
        const float matrixY = (y + .5f) * stepY;
        const int row = std::min(rows - 1, int(matrixY));
        const float dy = std::abs(matrixY - row - .5f);
        for (int x = 0; x < output.width(); ++x) {
            const float matrixX = (x + .5f) * stepX;
            const int column = std::min(columns - 1, int(matrixX));
            const float dx = std::abs(matrixX - column - .5f);
            const float distance = round ? std::sqrt(dx * dx + dy * dy) - Radius
                : std::max(dx - halfWidth, dy - Radius);
            const float coverage = std::lerp(openArea, 1.f - matrixSmoothStep(-edge, edge, distance), detail);
            const auto& cell = cells[size_t(row) * columns + column];
            auto& color = colors[size_t(y) * output.width() + x];
            for (size_t channel = 0; channel < color.size(); ++channel) {
                const float base = background[channel];
                color[channel] = emissive ? std::lerp(EmissiveBlack, cell.active[channel], coverage)
                    : std::lerp(base, std::lerp(base * inactiveGain, cell.active[channel], cell.level), coverage);
            }
        }
    }
}

}
