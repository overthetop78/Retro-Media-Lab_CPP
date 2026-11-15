#pragma once
#include "../FormatIds.h"
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Rx02::Constants {
inline constexpr std::uint32_t Tracks=77,Heads=1,PhysicalSectors=26,PhysicalSize=256,LogicalSize=512,Parts=LogicalSize/PhysicalSize,PhysicalCount=Tracks*PhysicalSectors,LogicalCount=PhysicalCount/Parts,LogicalPerTrack=PhysicalSectors/Parts,Capacity=PhysicalCount*PhysicalSize,Interleave=2,HalfOffset=1,Skew=6,FirstSector=1;
inline constexpr std::string_view FormatId=Formats::DecRx02,Extension=".img";
inline constexpr std::string_view WriterId="dec-rx02";
}
