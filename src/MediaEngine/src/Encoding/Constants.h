#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Encoding::Constants {
inline constexpr std::int32_t MaximumCylinder=255,MaximumHead=1,MinimumSectors=1;
inline constexpr std::uint32_t TickNanoseconds=25;
inline constexpr std::int32_t RequireCompletePhysicalTrack=1;
inline constexpr std::uint32_t DefaultCellTicks=40,DefaultIndexTicks=8000000;
inline constexpr std::uint64_t MaximumBits=2147483647;
inline constexpr int BitsPerByte=8,DoubleCells=2,QuadCells=4;
inline constexpr char Zero='0',One='1',HexUpperFirst='A',HexUpperLast='F',HexLowerFirst='a',HexLowerLast='f',HexDigitLast='9';
inline constexpr int HexLetterOffset=10,HexBase=16;
inline constexpr std::string_view CylinderField="Cylinder",HeadField="Head",NumberField="Number";
}
