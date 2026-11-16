#pragma once
#include <array>
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::IsoReconstruction::RuntimeConstants {
inline constexpr std::array<std::string_view,7> DosOems{"IBM","MSDOS","MSWIN","DOS","FRDOS","FREEDOS","COPYDISK"};
inline constexpr std::string_view AtariScpPrefix="atari.scp.";
inline constexpr std::string_view AtrExtension=".atr",StExtension=".st";
inline constexpr char Separator='.',Null='\0',Space=' ';
inline constexpr std::int32_t FatFirstSector=2,Valid=2,Unknown=1,Invalid=0;
}
