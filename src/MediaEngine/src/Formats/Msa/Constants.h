#pragma once
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Msa::Constants {
inline constexpr std::uint16_t Signature=0x0e0f;
inline constexpr std::uint8_t Marker=0xe5;
inline constexpr std::size_t HeaderSize=10,SignatureOffset=0,SectorsOffset=2,HeadsOffset=4,StartOffset=6,EndOffset=8,WordSize=2,RunSize=4,ValueOffset=1,CountOffset=2;
inline constexpr std::uint32_t SectorSize=512,MinimumSectors=1,MaximumSectors=36,MinimumHeads=1,MaximumHeads=2,MaximumCylinder=255,MaximumRun=65535,BitsPerByte=8,FirstSector=1;
inline constexpr std::size_t MaxLength=2147483647;
}
