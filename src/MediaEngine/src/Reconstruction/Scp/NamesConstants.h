#pragma once
#include "Constants.h"
#include <array>
#include <utility>
namespace RetroMediaLab::MediaEngine::ScpReconstruction::NamesConstants {
using Name=std::pair<std::string_view,std::string_view>;
inline constexpr std::array<Name,8> Exact{{
    {Constants::Amiga,"Amiga"},{Constants::IsoAutomatic,"ISO MFM/FM"},
    {Constants::Atari,"Atari ST"},{Constants::AtariSt720,"Atari ST"},
    {Constants::CommodoreAutomatic,"Commodore"},{Constants::Commodore1581,"Commodore 1581"},
    {Constants::Apple,"Apple"},{Constants::Dec,"DEC RX02"}}};
inline constexpr std::array<Name,6> Patterns{{
    {"ibm","IBM PC"},{"amstrad.cpc","Amstrad CPC"},{"amstrad.pcw","Amstrad PCW"},
    {"acorn","Acorn"},{"epson","Epson QX-10"},{"ucsd","UCSD p-System"}}};
inline constexpr std::array<std::string_view,5> Families{"Iso","Amiga","Commodore","Apple","Dec"};
}
