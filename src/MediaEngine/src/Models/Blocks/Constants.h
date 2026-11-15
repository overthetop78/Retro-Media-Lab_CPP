#pragma once
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Blocks {
inline constexpr std::size_t CopyBufferSize=65536,SearchDivisor=2;
inline constexpr std::string_view CylindersParameter="cylinders",HeadsParameter="heads",SectorsParameter="sectorsPerTrack",SectorSizeParameter="sectorSize",StoredName="Stored",ZeroName="Zero",UnallocatedName="Unallocated",UnavailableName="Unavailable";
}
