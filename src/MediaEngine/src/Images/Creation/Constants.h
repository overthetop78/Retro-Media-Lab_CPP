#pragma once
#include <RetroMediaLab/MediaEngine/Migration.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include "../../Formats/FormatIds.h"
#include "../../Formats/Adf/Constants.h"
#include "../../Formats/Apple/Constants.h"
#include "../../Formats/Raw/Constants.h"
#include "../../Formats/CommodoreDisks/Constants.h"
#include <array>
namespace RetroMediaLab::MediaEngine::CreationConstants {
namespace F = Formats;
namespace S = MediaFileSystems::FileSystemIds;
inline constexpr std::string_view StExtension = ".st", GeneratedSource = "migration";
inline constexpr std::size_t CatalogueHigh = 6, CatalogueLow = 7;
inline constexpr int ByteShift = 8;
inline constexpr std::array<FileSystemMigrationTarget,11> Targets{{
    {F::AmigaDos,S::AmigaDosFfs,Adf::Constants::AdfExtension},
    {F::AtariSt720,S::Fat12,StExtension},
    {F::Ibm720,S::Fat12,Raw::Constants::Img},
    {F::Msx2Dd,S::Fat12,Raw::Constants::Dsk},
    {F::AppleIIAppleDos140,S::AppleDos,Apple::Constants::Do},
    {F::AppleIIProDos140,S::ProDos,Apple::Constants::Po},
    {F::AppleIIProDos800,S::ProDos,Apple::Constants::TwoMg},
    {F::AppleIIISos,S::Sos,Apple::Constants::TwoMg},
    {F::Commodore1541,S::CommodoreDos,CommodoreDisks::Constants::D64Extension},
    {F::Commodore1571,S::CommodoreDos,CommodoreDisks::Constants::D71Extension},
    {F::Commodore1581,S::CommodoreDos,CommodoreDisks::Constants::D81Extension}
}};
}
