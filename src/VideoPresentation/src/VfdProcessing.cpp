#include "VfdProcessing.h"
#include "MatrixCells.h"

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {

LinearColor vfdPhosphor(VfdColor color) {
    switch (color) {
    case VfdColor::Green: return {.055f, 1.f, .12f};
    case VfdColor::Blue: return {.055f, .29f, 1.f};
    case VfdColor::White: return {1.f, 1.f, 1.f};
    case VfdColor::Amber: return {1.f, .42f, .028f};
    case VfdColor::Red: return {1.f, .045f, .025f};
    default: return {.085f, 1.f, .73f};
    }
}

void applyVfd(std::vector<LinearColor>& colors, QSize output, QSize source, const VfdProfile& profile) {
    if (colors.empty() || output.isEmpty() || source.isEmpty()) return;
    const auto grid = matrixSize(profile.definition, source);
    const int columns = grid.width(), rows = grid.height();
    std::vector<float> emission(size_t(columns) * rows);
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < columns; ++x)
            emission[size_t(y) * columns + x] = matrixLuminance(sampleMatrixCell(colors, source, grid, x, y));

    const auto phosphor = vfdPhosphor(profile.color);
    constexpr LinearColor Glass{.002f, .0045f, .0055f};
    constexpr float RoundRadius = .38f, RectangleHalfWidth = .28f;
    constexpr float RoundArea = .454f, SquareArea = .578f, RectangleArea = .426f;
    constexpr float MinimumEdge = .025f, MaximumEdge = .20f;
    constexpr float DetailStart = 1.f, DetailEnd = 2.5f;
    constexpr float MinimumGlowRadius = .035f, MaximumGlowRadius = .115f;
    constexpr float GlowGain = .24f, EmissionGain = .94f;
    constexpr float UnlitBase = .0018f, UnlitPhosphor = .004f;
    const bool round = profile.shape == DotMatrixShape::Round;
    const bool rectangle = profile.shape == DotMatrixShape::Rectangle;
    const float meanArea = round ? RoundArea : rectangle ? RectangleArea : SquareArea;
    const float halfWidth = rectangle ? RectangleHalfWidth : RoundRadius;
    const float stepX = float(columns) / output.width(), stepY = float(rows) / output.height();
    const float scale = 1.f / std::max(stepX, stepY);
    const float edge = std::clamp(.5f / std::max(scale, 1.f), MinimumEdge, MaximumEdge);
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
            const float distance = round ? std::sqrt(dx * dx + dy * dy) - RoundRadius
                : std::max(dx - halfWidth, dy - RoundRadius);
            const float level = emission[size_t(row) * columns + column];
            const float core = 1.f - matrixSmoothStep(-edge, edge, distance);
            const float coverage = std::lerp(meanArea, core, detail);
            const float radius = std::lerp(MinimumGlowRadius, MaximumGlowRadius, level);
            const float glow = (1.f - matrixSmoothStep(0.f, radius, std::max(distance, 0.f))) *
                (1.f - core) * (GlowGain * level * level) * detail;
            auto& color = colors[size_t(y) * output.width() + x];
            for (size_t channel = 0; channel < color.size(); ++channel) {
                const float unlit = UnlitBase + phosphor[channel] * UnlitPhosphor;
                color[channel] = std::min(1.f, Glass[channel] + unlit * coverage +
                    phosphor[channel] * (EmissionGain * level * coverage + glow));
            }
        }
    }
}

}
