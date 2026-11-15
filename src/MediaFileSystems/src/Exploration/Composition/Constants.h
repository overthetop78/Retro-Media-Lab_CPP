#pragma once
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::Composition {
inline constexpr char Null='\0',EntrySeparator='\x1f',PathSeparator='/',FormatSeparator='.';
inline constexpr std::string_view DirectoryKind="Directory",FileKind="File",LinkKind="Link",UnknownKind="Unknown";
inline constexpr char32_t AsciiMaximum=0x7f,SurrogateOffset=0x10000,HighSurrogate=0xd800,LowSurrogate=0xdc00,SurrogateMask=0x3ff;
inline constexpr unsigned SurrogateShift=10;
}
