#pragma once
#include "ProtectedTrack.h"
namespace RetroMediaLab::MediaEngine::Flux {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint32_t> toIntervals(BitView,std::uint32_t);
RETROMEDIALAB_MEDIAENGINE_API Bits toBits(Intervals,std::uint32_t indexTicks,std::uint32_t cellTicks);
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t greatestCommonDivisor(Intervals) noexcept;
struct NanosecondRevolution {std::uint32_t indexTime=0;std::vector<std::uint32_t> intervals;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<NanosecondRevolution> readFlux(const ProtectedTrack&);
}
