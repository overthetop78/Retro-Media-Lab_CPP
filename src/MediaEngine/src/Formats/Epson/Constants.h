#pragma once
#include "Geometry.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
namespace RetroMediaLab::MediaEngine::Epson::Constants {
using namespace MediaFileSystems::Formats;
inline constexpr std::int32_t BooterTracks=15,Small399=1,Small396=4,SmallLogo=6,Cylinders=40,SingleHead=1,DoubleHeads=2;
inline constexpr TrackGeometry Layout320{1,16,256},Layout400{1,5,1024},LayoutBooter{1,17,256},LayoutData{1,10,512},LayoutLogo{2,10,512};
inline constexpr std::int32_t FirstCylinder=0,SecondCylinder=1,LogoSmallCylinder=4,LogoAlternateFirst=5,LogoAlternateLast=6,LogoEmptyFirst=3,LogoEmptyLast=7;
inline constexpr std::string_view WriterId="epson-qx10";
inline constexpr std::string_view RawExtension=".img";
inline constexpr std::string_view Separator=", ";
}
