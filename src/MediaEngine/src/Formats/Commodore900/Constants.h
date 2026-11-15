#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Commodore900::Constants {
inline constexpr std::uint32_t CylinderCount=80,HeadCount=2,SectorSize=512,MaximumSectors=16,MinimumSectors=13,HeaderSize=1024,MinimumBlocks=3;
inline constexpr std::array<std::uint32_t,4> ZoneEnds={39,53,64,CylinderCount},ZoneSectors={16,15,14,13};
inline constexpr std::uint32_t BlockCount=39*2*16+(53-39)*2*15+(64-53)*2*14+(80-64)*2*13;
inline constexpr std::uint64_t Capacity=std::uint64_t(BlockCount)*SectorSize;
inline constexpr std::string_view WriterId="coherent-raw",BinExtension=".bin",ImgExtension=".img";
inline constexpr std::string_view FormatId="commodore900.coherent";
inline constexpr char UpperFirst='A',UpperLast='Z';
inline constexpr int CaseDelta='a'-'A';
}
