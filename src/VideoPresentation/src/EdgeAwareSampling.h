#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace RetroMediaLab::VideoPresentation::SoftwareProcessing::EdgeAwareSampling {

using Color = std::array<float, 3>;

inline Color mix(const Color& a, float aw, const Color& b, float bw, const Color& c = {}, float cw = 0) {
    Color result{};
    for (std::size_t n = 0; n < result.size(); ++n)
        result[n] = (a[n]*aw + b[n]*bw + c[n]*cw) / (aw+bw+cw);
    return result;
}

inline Color interpolate(const Color& a, const Color& b, float amount) {
    Color result{};
    for (std::size_t n = 0; n < result.size(); ++n) result[n] = a[n] + (b[n] - a[n]) * amount;
    return result;
}

inline bool hqDifferent(const Color& a, const Color& b) {
    const float r = a[0]-b[0], g = a[1]-b[1], blue = a[2]-b[2];
    return std::abs(.25f*(r+g+blue)) > .018f || std::abs(.25f*(r-blue)) > .002f ||
        std::abs(-.125f*r+.25f*g-.125f*blue) > .005f;
}

// HQx rule order from the C# port of mGBA's HQ2x shader. See THIRD-PARTY-NOTICES.md.
template<class Read>
Color hqx(const Read& read, int x, int y, float fx, float fy) {
    const int dx = fx > .5f ? -1 : 1, dy = fy > .5f ? -1 : 1;
    const auto& w0 = read(x-dx,y-dy); const auto& w1 = read(x,y-dy);
    const auto& w2 = read(x+dx,y-dy); const auto& w3 = read(x-dx,y);
    const auto& w4 = read(x,y); const auto& w5 = read(x+dx,y);
    const auto& w6 = read(x-dx,y+dy); const auto& w7 = read(x,y+dy);
    const auto& w8 = read(x+dx,y+dy);
    int pattern = 0, bit = 1;
    for (const auto* neighbor : {&w0,&w1,&w2,&w3,&w5,&w6,&w7,&w8}) {
        if (hqDifferent(*neighbor, w4)) pattern |= bit;
        bit <<= 1;
    }
    const auto p = [pattern](int mask, int value) { return (pattern & mask) == value; };
    const bool leftEdge = p(0xBF,0x37) || p(0xDB,0x13);
    const bool topEdge = p(0xDB,0x49) || p(0xEF,0x6D);
    if (leftEdge && hqDifferent(w1,w5)) return mix(w4,3,w3,1);
    if (topEdge && hqDifferent(w7,w3)) return mix(w4,3,w1,1);
    if ((p(0x0B,0x0B) || p(0xFE,0x4A) || p(0xFE,0x1A)) && hqDifferent(w3,w1)) return w4;
    if ((p(0x6F,0x2A) || p(0x5B,0x0A) || p(0xBF,0x3A) || p(0xDF,0x5A) || p(0x9F,0x8A) ||
        p(0xCF,0x8A) || p(0xEF,0x4E) || p(0x3F,0x0E) || p(0xFB,0x5A) || p(0xBB,0x8A) ||
        p(0x7F,0x5A) || p(0xAF,0x8A) || p(0xEB,0x8A)) && hqDifferent(w3,w1)) return mix(w4,3,w0,1);
    if (p(0x0B,0x08)) return mix(w4,2,w0,1,w1,1);
    if (p(0x0B,0x02)) return mix(w4,2,w0,1,w3,1);
    if (p(0x2F,0x2F)) return mix(w4,4,w3,1,w1,1);
    if (leftEdge) return mix(w4,5,w1,2,w3,1);
    if (topEdge) return mix(w4,5,w3,2,w1,1);
    if (p(0x1B,0x03) || p(0x4F,0x43) || p(0x8B,0x83) || p(0x6B,0x43)) return mix(w4,3,w3,1);
    if (p(0x4B,0x09) || p(0x8B,0x89) || p(0x1F,0x19) || p(0x3B,0x19)) return mix(w4,3,w1,1);
    if (p(0x7E,0x2A) || p(0xEF,0xAB) || p(0xBF,0x8F) || p(0x7E,0x0E)) return mix(w4,2,w3,3,w1,3);
    if (p(0xFB,0x6A) || p(0x6F,0x6E) || p(0x3F,0x3E) || p(0xFB,0xFA) || p(0xDF,0xDE) || p(0xDF,0x1E))
        return mix(w4,3,w0,1);
    if (p(0x0A,0x00) || p(0x4F,0x4B) || p(0x9F,0x1B) || p(0x2F,0x0B) || p(0xBE,0x0A) ||
        p(0xEE,0x0A) || p(0x7E,0x0A) || p(0xEB,0x4B) || p(0x3B,0x1B)) return mix(w4,2,w3,1,w1,1);
    return mix(w4,6,w3,1,w1,1);
}

inline float perceptualDistance(const Color& a, const Color& b) {
    const float redMean = .5f*(a[0]+b[0]);
    const float r = a[0]-b[0], g = a[1]-b[1], blue = a[2]-b[2];
    return std::sqrt((2.f+redMean)*r*r + 4.f*g*g + (3.f-redMean)*blue*blue) / 3.f;
}

struct Corner { Color color{}; float strength = 0.f; };
inline constexpr float ScaleFxCandidateThreshold = .5f;
inline constexpr float ScaleFxDifferenceThreshold = .06f;

inline Corner scaleFxCorner(const Color& center, const Color& h, const Color& v, const Color& diagonal,
    const Color& oppositeH, const Color& oppositeV) {
    const auto junction = perceptualDistance(h, v);
    const auto gapH = perceptualDistance(center, h), gapV = perceptualDistance(center, v);
    const auto gap = std::min(gapH, gapV);
    if (junction >= ScaleFxCandidateThreshold || gap <= ScaleFxDifferenceThreshold ||
        perceptualDistance(h, oppositeH) <= ScaleFxDifferenceThreshold ||
        perceptualDistance(v, oppositeV) <= ScaleFxDifferenceThreshold) return {};
    const auto continuation = std::min(perceptualDistance(diagonal, h), perceptualDistance(diagonal, v));
    const auto strength = std::clamp((ScaleFxCandidateThreshold-junction)/ScaleFxCandidateThreshold,0.f,1.f) *
        std::clamp((gap+continuation)/std::max(junction,.001f),0.f,1.f);
    return strength > ScaleFxDifferenceThreshold ? Corner{gapH <= gapV ? h : v, strength} : Corner{};
}

inline Color scaleFxMid(const Color& center, const Color& axial, Corner first, Corner second) {
    if (first.strength > 0 && second.strength > 0 && perceptualDistance(first.color, second.color) < ScaleFxCandidateThreshold)
        return perceptualDistance(axial, first.color) <= perceptualDistance(axial, second.color) ? first.color : second.color;
    const auto& best = first.strength >= second.strength ? first : second;
    return best.strength > 0 && perceptualDistance(best.color, axial) < ScaleFxCandidateThreshold*.5f ? axial : center;
}

// ScaleFX perceptual selection from the C# port. See THIRD-PARTY-NOTICES.md.
template<class Read>
Color scaleFx(const Read& at, float fx, float fy) {
    const auto& e = at(0,0);
    const int cell = std::clamp(int(fy*3),0,2)*3 + std::clamp(int(fx*3),0,2);
    if (cell == 4) return e;
    const auto& b = at(0,-1); const auto& d = at(-1,0);
    const auto& f = at(1,0); const auto& h = at(0,1);
    const auto tl = [&] { return scaleFxCorner(e,d,b,at(-1,-1),h,f); };
    const auto tr = [&] { return scaleFxCorner(e,b,f,at(1,-1),h,d); };
    const auto bl = [&] { return scaleFxCorner(e,h,d,at(-1,1),b,f); };
    const auto br = [&] { return scaleFxCorner(e,f,h,at(1,1),d,b); };
    const auto color = [&e](Corner corner) { return corner.strength > 0 ? corner.color : e; };
    switch (cell) {
    case 0: return color(tl());
    case 1: return scaleFxMid(e,b,tl(),tr());
    case 2: return color(tr());
    case 3: return scaleFxMid(e,d,tl(),bl());
    case 5: return scaleFxMid(e,f,tr(),br());
    case 6: return color(bl());
    case 7: return scaleFxMid(e,h,bl(),br());
    default: return color(br());
    }
}

template<class Read>
Color sabr(const Read& at, float fx, float fy) {
    const auto& center = at(0,0);
    Corner best;
    const auto consider = [&](const Color& h, const Color& v, const Color& diagonal, float proximity) {
        if (proximity <= 0.f) return;
        const auto junction = perceptualDistance(h,v);
        const auto gapH = perceptualDistance(center,h), gapV = perceptualDistance(center,v);
        const auto contrast = std::clamp(std::min(gapH,gapV)/.2f,0.f,1.f);
        const auto coherence = 1.f-std::clamp(junction/.45f,0.f,1.f);
        const auto opposing = gapH+gapV+.001f;
        const auto energy = junction+.5f*perceptualDistance(center,diagonal);
        const auto dominance = std::clamp((opposing-energy)/opposing,0.f,1.f);
        const auto strength = .75f*std::min(proximity,1.f)*coherence*contrast*(.4f+.6f*dominance);
        if (strength > best.strength) best = {interpolate(h,v,.5f), strength};
    };
    consider(at(-1,0),at(0,-1),at(-1,-1),1.25f-1.5f*(fx+fy));
    consider(at(0,-1),at(1,0),at(1,-1),1.25f-1.5f*((1.f-fx)+fy));
    consider(at(1,0),at(0,1),at(1,1),1.25f-1.5f*((1.f-fx)+(1.f-fy)));
    consider(at(0,1),at(-1,0),at(-1,1),1.25f-1.5f*(fx+(1.f-fy)));
    return interpolate(center, best.color, best.strength);
}

inline float yuvDistance(const Color& a, const Color& b) {
    const float r = a[0]-b[0], g = a[1]-b[1], blue = a[2]-b[2];
    const float y = .299f*r+.587f*g+.114f*blue;
    const float cb = -.168736f*r-.331264f*g+.5f*blue;
    const float cr = .5f*r-.418688f*g-.081312f*blue;
    return 48.f*std::abs(y) + 7.f*std::abs(cb) + 6.f*std::abs(cr);
}

inline float smoothStep(float low, float high, float value) {
    const auto t = std::clamp((value-low)/(high-low),0.f,1.f);
    return t*t*(3.f-2.f*t);
}

// xBR level-1 rule from the C# port of Hyllian's shader. See THIRD-PARTY-NOTICES.md.
template<class Read>
Color xbr(const Read& at, float fx, float fy, bool useXbrz) {
    constexpr std::array directions{std::pair{1,1},std::pair{1,-1},std::pair{-1,-1},std::pair{-1,1}};
    const auto& center = at(0,0);
    Corner best;
    const auto luma = [](const Color& color) { return 48.f*(.299f*color[0]+.587f*color[1]+.114f*color[2]); };
    const auto yl = [&](int x,int y) { return luma(at(x,y)); };
    const auto distance = [](float a,float b,float c,float d,float e,float f,float g,float h) {
        return std::abs(a-b)+std::abs(a-c)+std::abs(d-e)+std::abs(d-f)+4.f*std::abs(g-h);
    };
    for (const auto& [dx,dy] : directions) {
        const auto& h = at(dx,0); const auto& v = at(0,dy);
        const float toward = (dx > 0 ? fx : 1.f-fx) + (dy > 0 ? fy : 1.f-fy);
        float gapH, gapV, blend;
        if (useXbrz) {
            gapH = yuvDistance(center,h);
            gapV = yuvDistance(center,v);
            if (gapH <= .01f || gapV <= .01f) continue;
            const auto& diagonal = at(dx,dy);
            const auto edge = yuvDistance(center,at(dx,-dy)) + yuvDistance(center,at(-dx,dy)) +
                yuvDistance(v,diagonal) + yuvDistance(h,diagonal) + 4.f*yuvDistance(v,h);
            const auto alternative = yuvDistance(v,at(-dx,0)) + yuvDistance(h,at(0,-dy)) +
                4.f*yuvDistance(center,diagonal);
            if (edge >= alternative) continue;
            blend = edge*1.5f < alternative ? smoothStep(.9f,1.7f,toward) : smoothStep(1.2f,1.9f,toward);
        } else {
            const auto e = luma(center), fl = luma(h), hl = luma(v);
            gapH = std::abs(e-fl);
            gapV = std::abs(e-hl);
            if (gapH <= .000001f || gapV <= .000001f) continue;
            const auto i = yl(dx,dy);
            const auto edge = distance(e,yl(dx,-dy),yl(-dx,dy),i,yl(0,2*dy),yl(2*dx,0),hl,fl);
            const auto alternative = distance(hl,yl(-dx,0),yl(dx,2*dy),fl,yl(2*dx,dy),yl(0,-dy),e,i);
            if (edge >= alternative) continue;
            blend = smoothStep(1.1f,1.9f,toward);
        }
        if (blend > best.strength) best = {gapH <= gapV ? h : v, blend};
    }
    return interpolate(center, best.color, best.strength);
}

} // namespace RetroMediaLab::VideoPresentation::SoftwareProcessing::EdgeAwareSampling
