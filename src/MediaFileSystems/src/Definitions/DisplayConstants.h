#pragma once
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <array>
#include <cstdint>
#include <string_view>
#include <utility>
namespace RetroMediaLab::MediaFileSystems::DisplayConstants {
inline constexpr std::string_view Domain="MediaFileSystems",FirstParameter="{0}",SecondParameter="{1}";
inline constexpr std::string_view IbmPrefix="ibm.",MsxPrefix="msx.";
inline constexpr int HexBase=16,VersionWidth=4;
inline constexpr std::uint32_t AppleDos32Sectors=13;
inline constexpr char HexFill='0';
inline constexpr std::uint16_t LisaTableVersion=0x000e,LisaHashVersion=0x000f,LisaTreeVersion=0x0011;
inline constexpr std::string_view AcornAdfs="MediaFileSystems.Name.AcornAdfs";
inline constexpr std::string_view AcornDfs="MediaFileSystems.Name.AcornDfs";
inline constexpr std::string_view AppleDos32="MediaFileSystems.Name.AppleDos32";
inline constexpr std::string_view AppleDos33="MediaFileSystems.Name.AppleDos33";
inline constexpr std::string_view AtariDos="MediaFileSystems.Name.AtariDos";
inline constexpr std::string_view AtariClk="MediaFileSystems.Name.AtariClk";
inline constexpr std::string_view AtariBoot="MediaFileSystems.Name.AtariBoot";
inline constexpr std::string_view AtariProduct="MediaFileSystems.Name.AtariProduct";
inline constexpr std::string_view AtariClkProduct="MediaFileSystems.Name.AtariClkProduct";
inline constexpr std::string_view AtariKFile="MediaFileSystems.Name.AtariKFile";
inline constexpr std::string_view AtariMyDos="MediaFileSystems.Name.AtariMyDos";
inline constexpr std::string_view AtariSpartaDos="MediaFileSystems.Name.AtariSpartaDos";
inline constexpr std::string_view Coherent="MediaFileSystems.Name.Coherent";
inline constexpr std::string_view CommodoreDos="MediaFileSystems.Name.CommodoreDos";
inline constexpr std::string_view Cpm="MediaFileSystems.Name.Cpm";
inline constexpr std::string_view MacHfs="MediaFileSystems.Name.MacHfs";
inline constexpr std::string_view MacMfs="MediaFileSystems.Name.MacMfs";
inline constexpr std::string_view Rt11="MediaFileSystems.Name.Rt11";
inline constexpr std::string_view Ucsd="MediaFileSystems.Name.Ucsd";
inline constexpr std::string_view AmigaDos="MediaFileSystems.Name.AmigaDos";
inline constexpr std::string_view AmigaDosOfs="MediaFileSystems.Name.AmigaDosOfs";
inline constexpr std::string_view AmigaDosFfs="MediaFileSystems.Name.AmigaDosFfs";
inline constexpr std::string_view AmigaDosOfsInternational="MediaFileSystems.Name.AmigaDosOfsInternational";
inline constexpr std::string_view AmigaDosFfsInternational="MediaFileSystems.Name.AmigaDosFfsInternational";
inline constexpr std::string_view AmigaDosOfsDirectoryCache="MediaFileSystems.Name.AmigaDosOfsDirectoryCache";
inline constexpr std::string_view AmigaDosFfsDirectoryCache="MediaFileSystems.Name.AmigaDosFfsDirectoryCache";
inline constexpr std::string_view AmigaDosOfsLongNames="MediaFileSystems.Name.AmigaDosOfsLongNames";
inline constexpr std::string_view AmigaDosFfsLongNames="MediaFileSystems.Name.AmigaDosFfsLongNames";
inline constexpr std::string_view AmstradPcw="MediaFileSystems.Name.AmstradPcw";
inline constexpr std::string_view AmstradCpc="MediaFileSystems.Name.AmstradCpc";
inline constexpr std::string_view Fat12Ibm="MediaFileSystems.Name.Fat12Ibm";
inline constexpr std::string_view Fat12Msx="MediaFileSystems.Name.Fat12Msx";
inline constexpr std::string_view Fat12Atari="MediaFileSystems.Name.Fat12Atari";
inline constexpr std::string_view LisaProduct="MediaFileSystems.Name.LisaProduct";
inline constexpr std::string_view LisaTable="MediaFileSystems.Name.LisaTable";
inline constexpr std::string_view LisaHash="MediaFileSystems.Name.LisaHash";
inline constexpr std::string_view LisaTree="MediaFileSystems.Name.LisaTree";
inline constexpr std::string_view LisaTreeType="MediaFileSystems.Name.LisaTreeType";
inline constexpr std::string_view LisaUnknown="MediaFileSystems.Name.LisaUnknown";
inline constexpr std::string_view ProDos="MediaFileSystems.Name.ProDos";
inline constexpr std::string_view Sos="MediaFileSystems.Name.Sos";
inline constexpr std::array<std::pair<std::string_view,std::string_view>,24> Fixed{{
{FileSystemIds::AcornAdfs,AcornAdfs},
{FileSystemIds::AcornDfs,AcornDfs},
{FileSystemIds::AtariDos,AtariDos},
{FileSystemIds::AtariKFile,AtariKFile},
{FileSystemIds::AtariMyDos,AtariMyDos},
{FileSystemIds::AtariSpartaDos,AtariSpartaDos},
{FileSystemIds::Coherent,Coherent},
{FileSystemIds::CommodoreDos,CommodoreDos},
{FileSystemIds::Cpm,Cpm},
{FileSystemIds::MacHfs,MacHfs},
{FileSystemIds::MacMfs,MacMfs},
{FileSystemIds::Rt11,Rt11},
{FileSystemIds::Ucsd,Ucsd},
{FileSystemIds::AmigaDos,AmigaDos},
{FileSystemIds::AmigaDosOfs,AmigaDosOfs},
{FileSystemIds::AmigaDosFfs,AmigaDosFfs},
{FileSystemIds::AmigaDosOfsInternational,AmigaDosOfsInternational},
{FileSystemIds::AmigaDosFfsInternational,AmigaDosFfsInternational},
{FileSystemIds::AmigaDosOfsDirectoryCache,AmigaDosOfsDirectoryCache},
{FileSystemIds::AmigaDosFfsDirectoryCache,AmigaDosFfsDirectoryCache},
{FileSystemIds::AmigaDosOfsLongNames,AmigaDosOfsLongNames},
{FileSystemIds::AmigaDosFfsLongNames,AmigaDosFfsLongNames},
{FileSystemIds::ProDos,ProDos},
{FileSystemIds::Sos,Sos}
}};
}
