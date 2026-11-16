#pragma once
#include "../Formats/FormatIds.h"
#include <string_view>
#include <utility>
namespace RetroMediaLab::MediaEngine::Visualization::ClassificationConstants {
inline constexpr std::string_view Amstrad="Amstrad",Dec="DEC",AppleII="Apple II",RawScp="raw.scp",Rwts18Name="Brøderbund RWTS18";
inline constexpr std::string_view IsoMfm="iso.mfm",DensityMarker="hd",DensitySuffix="_hd",HighDensity="1440",ExtendedDensity="2880";
inline constexpr std::pair<std::string_view,std::string_view> DecoderByMachine[]{
 {AppleII,"apple2.gcr"},{"Apple Macintosh","applemac.gcr"},{"Apple Lisa","applelisa.fileware.gcr"},{"Amiga","amiga.mfm"},{"Commodore","commodore.gcr"},{Dec,Formats::DecRx02}
};
inline constexpr std::string_view ThreeHalfMachines[]{"Atari ST","Amiga","IBM PC","Apple Macintosh","MSX"};
inline constexpr std::string_view FiveQuarterMachines[]{AppleII,"Commodore","Acorn","Acorn / BBC Micro"};
inline constexpr std::pair<std::string_view,std::string_view> FormatAliases[]{
 {Formats::AppleIIDos33,Formats::AppleIIAppleDos140},{Formats::AppleIIRwts18,Formats::AppleIIAppleDos140},{Formats::AppleIIDos32,Formats::AppleIIAppleDos113},{Formats::AppleIIProDos,Formats::AppleIIProDos140},{Formats::AppleMacMfs,Formats::Mac400},{Formats::AppleMacHfs,Formats::Mac800}
};
inline constexpr std::string_view Dsk=".dsk",Edsk=".edsk",Atr=".atr";
}
