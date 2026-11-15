#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::AcornAtom::Constants {
inline constexpr std::uint32_t BlockSize=256,Cylinders=40,Heads=1,SectorsPerTrack=10,HeaderBlocks=2,FirstSector=0;
inline constexpr std::uint64_t Capacity=std::uint64_t(BlockSize)*Cylinders*Heads*SectorsPerTrack;
inline constexpr std::array<std::string_view,3> Extensions{".dsk",".ima",".img"};
inline constexpr std::string_view WriterId="acorn-atom-dsk";
inline constexpr std::string_view FormatId=MediaFileSystems::Formats::AcornAtomDos;
}
