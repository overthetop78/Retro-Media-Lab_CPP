#pragma once
#include <array>
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::RepresentationConversionConstants {
inline constexpr std::string_view FluxSectorsId="scp-flux-to-sectors",SectorsFluxId="sectors-to-scp-flux",HfeCellsId="flux-to-hfe-cells",SignalId="tape-signal",ClockParameter="clock";
inline constexpr char CoordinateSeparator=',';
inline constexpr double StartPercent=0,DecodeStartPercent=5,DecodePercentSpan=85,NanosecondsPerMicrosecond=1000,FmCellNanoseconds=4000;
inline constexpr std::array<double,4> NominalCells{500,1000,2000,4000};
inline constexpr std::int64_t MedianDivisor=2,RoundingParity=2,BitsPerByte=8,MaximumBits=2147483647;
inline constexpr double Half=0.5;
}
