#pragma once
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::FileSystemIds {
inline constexpr std::string_view Iso9660="iso9660";
inline constexpr std::string_view Joliet="iso9660-joliet";
inline constexpr std::string_view RockRidge="iso9660-rock-ridge";
inline constexpr std::string_view Udf="udf";
inline constexpr std::string_view Xdvdfs="microsoft-xdvdfs";
inline constexpr std::string_view NintendoGameCubeFst="nintendo-gamecube-fst";
inline constexpr std::string_view SequentialContent="sequential-content";
inline constexpr std::string_view AcornAdfs="acorn-adfs";
inline constexpr std::string_view AcornDfs="acorn-dfs";
inline constexpr std::string_view AmigaDos="amigados";
inline constexpr std::string_view AmigaDosOfs="amigados.ofs";
inline constexpr std::string_view AmigaDosFfs="amigados.ffs";
inline constexpr std::string_view AmigaDosOfsInternational="amigados.ofs-international";
inline constexpr std::string_view AmigaDosFfsInternational="amigados.ffs-international";
inline constexpr std::string_view AmigaDosOfsDirectoryCache="amigados.ofs-directory-cache";
inline constexpr std::string_view AmigaDosFfsDirectoryCache="amigados.ffs-directory-cache";
inline constexpr std::string_view AmigaDosOfsLongNames="amigados.ofs-long-names";
inline constexpr std::string_view AmigaDosFfsLongNames="amigados.ffs-long-names";
inline constexpr std::string_view AmigaFlatResourceArchive="amiga-flat-resource-archive";
inline constexpr std::string_view AmstradCpm="amstrad.cpm";
inline constexpr std::string_view AmstradMediaStructure="amstrad-media-structure";
inline constexpr std::string_view ConsoleCartridgeStructure="console-cartridge-structure";
inline constexpr std::string_view AppleDos="apple-dos";
inline constexpr std::string_view AtariDos="atari-dos";
inline constexpr std::string_view AtariClkGraphicsLibrary="atari-clk-graphics-library";
inline constexpr std::string_view AtariBootDisk="atari-boot-disk";
inline constexpr std::string_view AtariKFile="atari-k-file";
inline constexpr std::string_view AtariMyDos="atari-mydos";
inline constexpr std::string_view AtariSpartaDos="atari-spartados";
inline constexpr std::string_view Coherent="coherent";
inline constexpr std::string_view CommodoreDos="commodore-dos";
inline constexpr std::string_view Cpm="cpm";
inline constexpr std::string_view Fat12="fat12";
inline constexpr std::string_view Lisa="lisa";
inline constexpr std::string_view MacHfs="mac-hfs";
inline constexpr std::string_view MacMfs="mac-mfs";
inline constexpr std::string_view ProDos="prodos";
inline constexpr std::string_view Sos="apple3-sos";
inline constexpr std::string_view Rt11="rt11";
inline constexpr std::string_view Ucsd="ucsd";
}
