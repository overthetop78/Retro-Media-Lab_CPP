#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::Apple::Constants {
inline constexpr std::string_view MacGcr="applemac.gcr";
inline constexpr std::string_view Dc42=".dc42",Image=".image",Nib=".nib",Woz=".woz",NoRequestedFormat="none";
inline constexpr std::string_view Img=".img",Dsk=".dsk",Po=".po";
inline constexpr std::string_view D13=".d13",Do=".do",TwoMg=".2mg",TargetSeparator=" : ",GeometrySeparator=" / ";
inline constexpr std::string_view SectorWriterId="apple-sector",NibbleWriterId="apple-nibble",MacintoshWriterId="macintosh",LisaWriterId="lisa-disk-copy";
}
