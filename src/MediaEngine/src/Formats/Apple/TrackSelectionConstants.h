#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Apple::TrackSelectionConstants {
inline constexpr std::int32_t RwtsSectorCount=6,RwtsSectorSize=768,MinimumCredibleRwts18TrackCount=2;
inline constexpr std::int32_t DistinctSectorScoreWeight=100,IntegrityScoreWeight=10,InitialScore=-1;
inline constexpr std::string_view AppleIIGcr="apple2.gcr";
}
