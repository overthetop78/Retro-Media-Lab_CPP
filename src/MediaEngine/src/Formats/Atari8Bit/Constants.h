#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Atari8Bit::Constants {
inline constexpr std::uint32_t FirstHead=0,FirstSector=1,Heads=1,LinearCylinders=1,LinearSectorsPerCylinder=1,SingleSize=128,DoubleSize=256,ExtendedSize=512,BootCount=3,StandardSectors=18,EnhancedSectors=26,ExtendedSectors=28,Cylinders=40,StandardCount=720,TruncatedCount=512,EnhancedCount=1040,ExtendedCount=1120;
struct Profile {std::string_view id;std::uint32_t sectorSize,sectorCount;};
using namespace MediaFileSystems::Formats;
inline constexpr std::array<Profile,3> Profiles{{{Atari90,SingleSize,StandardCount},{Atari130,SingleSize,EnhancedCount},{Atari180,DoubleSize,StandardCount}}};
inline constexpr char FormatSeparator='.';
}
