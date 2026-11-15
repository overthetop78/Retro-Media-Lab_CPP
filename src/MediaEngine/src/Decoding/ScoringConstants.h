#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::Decoding::ScoringConstants {
inline constexpr double ValidSectorBaseScore=4,ValidSectorRatioWeight=1,ValidSectorConfidenceWeight=0.1,UnverifiedSectorBaseScore=3,InvalidSectorConfidenceWeight=0.01,RawFluxBaseScore=1,StructuredFluxBaseScore=2;
inline constexpr std::string_view RawDecoderId="raw";
}
