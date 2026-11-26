#pragma once

#include "PixelArtSampling.h"
#include "ColorTransfer.h"
#include "EdgeAwareSampling.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace RetroMediaLab::VideoPresentation {

inline bool usesSoftwareSampling(Sampling mode) {
    return mode != Sampling::Nearest && mode != Sampling::Linear;
}

namespace SoftwareProcessing {
inline bool usesPerceptualSampling(Sampling mode) {
    switch (mode) {
    case Sampling::Xbr:
    case Sampling::Xbrz:
    case Sampling::Hqx:
    case Sampling::Hq2x:
    case Sampling::Hq3x:
    case Sampling::Hq4x:
    case Sampling::ScaleFx:
    case Sampling::Sabr:
        return true;
    default:
        return false;
    }
}

namespace SamplingFilters {

inline float sharpCoordinate(float position, float scale) {
    const auto base = std::floor(position);
    return base + std::clamp((position - base - .5f) * scale + .5f, 0.f, 1.f);
}

inline float cubicWeight(float distance) {
    constexpr float coefficient = -.5f;
    const float value = std::abs(distance);
    if (value <= 1.f)
        return ((coefficient + 2.f) * value - (coefficient + 3.f)) * value * value + 1.f;
    if (value < 2.f)
        return coefficient * (((value - 5.f) * value + 8.f) * value - 4.f);
    return 0.f;
}

inline float sinc(float value) {
    const auto angle = std::numbers::pi_v<float> * value;
    return std::abs(value) < .0001f ? 1.f : std::sin(angle) / angle;
}

inline float jinc(float value) {
    if (std::abs(value) < .0001f) return .5f;
    const float half = value * .5f;
    float term = half, sum = term;
    for (int index = 1; index < 12; ++index) {
        term *= -(half * half) / (float(index) * float(index + 1));
        sum += term;
    }
    return sum / value;
}

} // namespace SamplingFilters

template<class Read>
std::array<float, 3> filterSample(Sampling mode, const Read& read, float x, float y, float scaleX, float scaleY) {
    using namespace SamplingFilters;
    switch (mode) {
    case Sampling::ScaleNx:
        return scaleNxSample(read, x + .5f, y + .5f, std::min(scaleX, scaleY) >= 2.5f);
    case Sampling::EpxScale2x:
        return scaleNxSample(read, x + .5f, y + .5f, false);
    case Sampling::TwoXSai:
    case Sampling::SuperTwoXSai:
    case Sampling::SuperEagle:
        return saiSample(mode, read, x + .5f, y + .5f);
    default:
        break;
    }
    if (mode == Sampling::SharpBilinear) {
        x = sharpCoordinate(x, scaleX);
        y = sharpCoordinate(y, scaleY);
    }
    const auto left = int(std::floor(x));
    const auto top = int(std::floor(y));
    std::array<float, 3> result{};
    if (mode == Sampling::Linear || mode == Sampling::SharpBilinear) {
        const auto& a = read(left, top);
        const auto& b = read(left + 1, top);
        const auto& c = read(left, top + 1);
        const auto& d = read(left + 1, top + 1);
        for (std::size_t n = 0; n < result.size(); ++n)
            result[n] = std::lerp(std::lerp(a[n], b[n], x - float(left)),
                std::lerp(c[n], d[n], x - float(left)), y - float(top));
        return result;
    }

    float total = 0.f;
    const int radius = mode == Sampling::Lanczos ? 3 : 2;
    for (int row = 1 - radius; row <= radius; ++row) {
        for (int column = 1 - radius; column <= radius; ++column) {
            const float dx = x - float(left + column), dy = y - float(top + row);
            float weight = 0.f;
            switch (mode) {
            case Sampling::Bicubic:
                weight = cubicWeight(dx) * cubicWeight(dy);
                break;
            case Sampling::Jinc2: {
                const auto distance = std::sqrt(dx * dx + dy * dy);
                if (distance >= 2.f) continue;
                const auto angle = std::numbers::pi_v<float> * distance;
                weight = jinc(angle) * jinc(angle * .5f);
                break;
            }
            case Sampling::Lanczos:
                if (std::abs(dx) >= 3.f || std::abs(dy) >= 3.f) continue;
                weight = sinc(dx) * sinc(dx / 3.f) * sinc(dy) * sinc(dy / 3.f);
                break;
            default:
                return read(left, top);
            }
            const auto& pixel = read(left + column, top + row);
            for (std::size_t n = 0; n < result.size(); ++n) result[n] += pixel[n] * weight;
            total += weight;
        }
    }
    if (total != 0.f) {
        for (auto& channel : result) channel = std::clamp(channel / total, 0.f, 1.f);
    }
    return result;
}

template<class Read, class ReadEncoded>
std::array<float, 3> filterPerceptualSample(Sampling mode, const Read& read, const ReadEncoded& encoded,
    float x, float y) {
    using namespace EdgeAwareSampling;
    const int left = int(std::floor(x + .5f)), top = int(std::floor(y + .5f));
    const float fx = x + .5f - float(left), fy = y + .5f - float(top);
    const auto at = [&](int dx, int dy) -> const auto& { return encoded(left + dx, top + dy); };
    std::array<float, 3> result{};
    float bilinearWeight = 0.f;
    switch (mode) {
    case Sampling::Xbr:
    case Sampling::Xbrz:
        result = xbr(at, fx, fy, mode == Sampling::Xbrz);
        break;
    case Sampling::ScaleFx:
        result = scaleFx(at, fx, fy);
        break;
    case Sampling::Sabr:
        result = sabr(at, fx, fy);
        break;
    default:
        result = hqx(encoded, left, top, fx, fy);
        if (mode == Sampling::Hq2x) bilinearWeight = .06f;
        if (mode == Sampling::Hq3x) bilinearWeight = .18f;
        if (mode == Sampling::Hq4x) bilinearWeight = .32f;
        break;
    }
    for (auto& channel : result) channel = linear(channel);
    if (bilinearWeight > 0.f) {
        const auto bilinear = filterSample(Sampling::Linear, read, x, y, 1.f, 1.f);
        for (std::size_t n = 0; n < result.size(); ++n)
            result[n] = std::clamp(result[n] * (1.f - bilinearWeight) + bilinear[n] * bilinearWeight, 0.f, 1.f);
    }
    return result;
}

} // namespace SoftwareProcessing
} // namespace RetroMediaLab::VideoPresentation
