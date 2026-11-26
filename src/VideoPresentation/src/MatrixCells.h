#pragma once

#include "AnalogSignal.h"

#include <algorithm>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {
inline float matrixLuminance(const LinearColor& c) { return c[0] * .2126f + c[1] * .7152f + c[2] * .0722f; }

class MatrixCells {
public:
    MatrixCells(const std::vector<LinearColor>& colors, QSize output, QSize source, int pitch)
        : columns_((source.width() - 1) / pitch + 1), rows_((source.height() - 1) / pitch + 1),
          stepX_(float(source.width()) / output.width() / pitch),
          stepY_(float(source.height()) / output.height() / pitch), averages_(size_t(columns_) * rows_) {
        std::vector<size_t> counts(averages_.size());
        for (int y = 0; y < output.height(); ++y) {
            const auto row = size_t(qint64(y) * source.height() / output.height() / pitch) * columns_;
            for (int x = 0; x < output.width(); ++x) {
                const auto cell = row + qint64(x) * source.width() / output.width() / pitch;
                const auto& color = colors[size_t(y) * output.width() + x];
                for (size_t c = 0; c < color.size(); ++c) averages_[cell][c] += color[c];
                ++counts[cell];
            }
        }
        for (size_t i = 0; i < averages_.size(); ++i)
            for (auto& value : averages_[i]) value /= float(std::max(size_t{1}, counts[i]));
    }

    std::vector<LinearColor>& averages() { return averages_; }

    struct Position { int index; float offset; };

    Position column(int pixel) const { return position(pixel, stepX_, columns_); }
    Position row(int pixel) const { return position(pixel, stepY_, rows_); }
    size_t index(Position column, Position row) const { return size_t(row.index) * columns_ + column.index; }

    float core(float distance, float radius) const {
        const float edge = std::max(stepX_, stepY_);
        const float t = std::clamp((radius + edge - distance) / (2.f * edge), 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

private:
    static Position position(int pixel, float step, int count) {
        const float coordinate = (pixel + .5f) * step;
        const int cell = int(coordinate);
        return {std::min(cell, count - 1), coordinate - float(cell) - .5f};
    }

    int columns_, rows_;
    float stepX_, stepY_;
    std::vector<LinearColor> averages_;
};
}
