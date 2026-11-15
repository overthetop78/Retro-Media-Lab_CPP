#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::RawFlux::Constants {
inline constexpr std::string_view Id="raw",SourceDisplayName="Flux brut",NameKey="MediaEngine.RawFluxName",LongKey="MediaEngine.RawFluxLong",ShortKey="MediaEngine.RawFluxShort";
inline constexpr double Confidence=0.05,LongIntervalMultiplier=10,ShortPulseRatio=0.55,Half=0.5,EvenDivisor=2;
inline constexpr std::int32_t MinimumCellCount=1,MaximumCellCount=64;
}
