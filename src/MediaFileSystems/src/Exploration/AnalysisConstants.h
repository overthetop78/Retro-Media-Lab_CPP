#pragma once
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::AnalysisConstants {
inline constexpr std::array MediaContentCategoryNames{std::string_view{"File"},std::string_view{"Text"},std::string_view{"Document"},std::string_view{"SourceCode"},std::string_view{"BasicProgram"},std::string_view{"BootProgram"},std::string_view{"Program"},std::string_view{"Executable"},std::string_view{"Command"},std::string_view{"System"},std::string_view{"ObjectCode"},std::string_view{"Data"},std::string_view{"Configuration"},std::string_view{"Library"},std::string_view{"Image"},std::string_view{"Audio"},std::string_view{"Rom"},std::string_view{"Media"},std::string_view{"Archive"},std::string_view{"DiskImage"},std::string_view{"Link"},std::string_view{"Font"}};
inline constexpr std::array MediaExecutionKindNames{std::string_view{"None"},std::string_view{"NativeExecutable"},std::string_view{"InterpretedProgram"},std::string_view{"CommandScript"}};
inline constexpr std::array MediaContentFormatNames{std::string_view{"Unknown"},std::string_view{"Empty"},std::string_view{"PlainText"},std::string_view{"BasicProgram"},std::string_view{"NativeExecutable"},std::string_view{"CommandScript"},std::string_view{"ObjectCode"},std::string_view{"SystemBinary"},std::string_view{"StructuredData"},std::string_view{"Configuration"},std::string_view{"Library"},std::string_view{"Image"},std::string_view{"Audio"},std::string_view{"Rom"},std::string_view{"Media"},std::string_view{"Archive"},std::string_view{"DiskImage"},std::string_view{"Font"}};
inline constexpr std::array MediaTextEncodingNames{std::string_view{"NotApplicable"},std::string_view{"Unknown"},std::string_view{"Ascii"},std::string_view{"DosOem"},std::string_view{"Atascii"},std::string_view{"Petscii"},std::string_view{"Latin1"},std::string_view{"MacRoman"},std::string_view{"AppleAscii"},std::string_view{"Msx"}};
inline constexpr std::array MediaPreviewKindNames{std::string_view{"None"},std::string_view{"Text"},std::string_view{"BasicListing"},std::string_view{"Image"},std::string_view{"Audio"},std::string_view{"Hexadecimal"},std::string_view{"ExternalApplication"}};
inline constexpr std::string_view Prefix0 = "acorn.dfs";
inline constexpr std::string_view Prefix1 = "dec.";
inline constexpr std::string_view Prefix2 = "msx.";
inline constexpr std::string_view Prefix3 = "ucsd.";
inline constexpr std::string_view Prefix4 = "amiga.";
inline constexpr std::string_view Prefix5 = "ibm.";
inline constexpr std::string_view Prefix6 = "atarist.";
inline constexpr std::string_view Prefix7 = "atari.";
inline constexpr std::string_view Prefix8 = "apple2.dos";
inline constexpr std::string_view Prefix9 = "apple2.appledos";
inline constexpr std::string_view Prefix10 = "apple2.prodos";
inline constexpr std::string_view Prefix11 = "apple3.";
inline constexpr std::string_view Prefix12 = "applemac";
inline constexpr std::string_view Prefix13 = "mac.";
inline constexpr std::string_view Prefix14 = "applelisa";
inline constexpr std::string_view Prefix15 = "commodore.";
inline constexpr std::string_view AtariTape = "tape.atari-cas", CpmFileSystem = "cp/m";
}
