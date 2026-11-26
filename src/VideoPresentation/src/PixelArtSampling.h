#pragma once

#include <RetroMediaLab/VideoPresentation/Presentation.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing {

template<class Read>
std::array<float, 3> scaleNxSample(const Read& read, float x, float y, bool scale3) {
    const auto left = int(std::floor(x)), top = int(std::floor(y));
    const auto& b = read(left, top - 1);
    const auto& d = read(left - 1, top);
    const auto& e = read(left, top);
    const auto& f = read(left + 1, top);
    const auto& h = read(left, top + 1);
    if (b == h || d == f) return e;

    const int cells = scale3 ? 3 : 2;
    const int cellX = std::clamp(int((x - float(left)) * float(cells)), 0, cells - 1);
    const int cellY = std::clamp(int((y - float(top)) * float(cells)), 0, cells - 1);
    if (!scale3) {
        switch (cellY * cells + cellX) {
        case 0: return d == b ? d : e;
        case 1: return b == f ? f : e;
        case 2: return d == h ? d : e;
        case 3: return h == f ? f : e;
        default: return e;
        }
    }
    const auto& a = read(left - 1, top - 1);
    const auto& c = read(left + 1, top - 1);
    const auto& g = read(left - 1, top + 1);
    const auto& i = read(left + 1, top + 1);
    switch (cellY * cells + cellX) {
    case 0: return d == b ? d : e;
    case 1: return ((d == b && e != c) || (b == f && e != a)) ? b : e;
    case 2: return b == f ? f : e;
    case 3: return ((d == b && e != g) || (d == h && e != a)) ? d : e;
    case 5: return ((b == f && e != i) || (h == f && e != c)) ? f : e;
    case 6: return d == h ? d : e;
    case 7: return ((d == h && e != i) || (h == f && e != g)) ? h : e;
    case 8: return h == f ? f : e;
    default: return e;
    }
}

inline bool similarSaiColors(const std::array<float, 3>& a, const std::array<float, 3>& b) {
    return std::abs(a[0] - b[0]) * .299f + std::abs(a[1] - b[1]) * .587f +
        std::abs(a[2] - b[2]) * .114f < .035f;
}

template<class Read>
std::array<float, 3> saiSample(Sampling mode, const Read& read, float x, float y) {
    const auto left = int(std::floor(x)), top = int(std::floor(y));
    const float fx = x - float(left), fy = y - float(top);
    const int dx = fx < .5f ? -1 : 1, dy = fy < .5f ? -1 : 1;
    const auto& center = read(left, top);
    const auto& horizontal = read(left + dx, top);
    const auto& vertical = read(left, top + dy);
    const auto& diagonal = read(left + dx, top + dy);
    const bool similar = similarSaiColors(horizontal, vertical);
    const bool useDiagonal = !similar && mode == Sampling::SuperEagle && similarSaiColors(diagonal, horizontal);
    const float cornerX = fx - (dx < 0 ? 0.f : 1.f), cornerY = fy - (dy < 0 ? 0.f : 1.f);
    const float strength = mode == Sampling::TwoXSai ? .45f : mode == Sampling::SuperTwoXSai ? .62f : .78f;
    const float corner = std::clamp(1.f - 1.7f * std::sqrt(cornerX * cornerX + cornerY * cornerY), 0.f, 1.f) * strength;
    std::array<float, 3> result{};
    for (std::size_t channel = 0; channel < result.size(); ++channel) {
        float target;
        if (similar) target = horizontal[channel];
        else if (useDiagonal) target = diagonal[channel];
        else if (mode == Sampling::TwoXSai) target = (horizontal[channel] + vertical[channel]) / 2.f;
        else target = (horizontal[channel] + vertical[channel] + diagonal[channel]) / 3.f;
        result[channel] = center[channel] + (target - center[channel]) * corner;
    }
    return result;
}

} // namespace RetroMediaLab::VideoPresentation::SoftwareProcessing
