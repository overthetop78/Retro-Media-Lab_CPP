#pragma once
#include "Constants.h"
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Rx02 {
// Internal callers validate the full image/geometry and pass an in-range logical sector.
inline std::size_t physicalOffset(std::uint32_t logical){using namespace Constants;const auto track=logical/PhysicalSectors;auto position=logical%PhysicalSectors;position=(Interleave*position+(position>=LogicalPerTrack?HalfOffset:0))%PhysicalSectors;const auto sector=(position+Skew*track)%PhysicalSectors;return std::size_t((track+1)%Tracks*PhysicalSectors+sector)*PhysicalSize;}
}
