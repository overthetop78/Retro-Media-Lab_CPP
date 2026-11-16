#pragma once
#include "../../Formats/FormatIds.h"
#include <RetroMediaLab/MediaEngine/DiskMetadataIds.h>
#include <array>
#include <limits>
namespace RetroMediaLab::MediaEngine::DiskMetadata::Constants {
inline constexpr std::string_view LisaAliasPrefix="lisa.",DecPrefix="dec.";
inline constexpr std::string_view CrackedBy="CRACKED BY",TheCompany="THE COMPANY",AtnSignature="ATN!",DosSignature="DOS";
inline constexpr std::size_t MaximumSignatureGap=128,BootBlockLength=1024,BootPayloadOffset=12,MinimumBootPayloadBytes=64,AtnHeaderSize=12,AtnExpandedOffset=4,AtnCompressedOffset=8;
inline constexpr std::uint32_t Percent=100,MinimumOccupiedPercentage=50,MinimumAvailablePercentage=95,MinimumAtnMembers=2,MaximumExpansionRatio=1024,ByteBits=8;
inline constexpr auto MaximumFlattenLength=std::numeric_limits<std::int32_t>::max();
struct SystemRule {std::string_view format,id;bool exact=false;};
inline constexpr std::array SystemRules{
 SystemRule{Formats::AppleMacPrefix,DiskSystemIds::Macintosh},SystemRule{Formats::MacPrefix,DiskSystemIds::Macintosh},
 SystemRule{Formats::AppleLisaPrefix,DiskSystemIds::Lisa},SystemRule{LisaAliasPrefix,DiskSystemIds::Lisa},
 SystemRule{Formats::AppleIIPrefix,DiskSystemIds::AppleII},SystemRule{Formats::AppleIIIPrefix,DiskSystemIds::AppleIII},
 SystemRule{Formats::AmigaPrefix,DiskSystemIds::Amiga},SystemRule{Formats::AtariStPrefix,DiskSystemIds::AtariSt},SystemRule{Formats::AtariPrefix,DiskSystemIds::Atari8Bit},
 SystemRule{Formats::IbmPrefix,DiskSystemIds::IbmPc},SystemRule{Formats::Commodore900Prefix,DiskSystemIds::Coherent},SystemRule{Formats::CommodorePrefix,DiskSystemIds::Commodore},SystemRule{Formats::AmstradPrefix,DiskSystemIds::Amstrad},
 SystemRule{Formats::AcornDosPlus800,DiskSystemIds::AcornBbc,true},SystemRule{Formats::AcornAdfsPrefix,DiskSystemIds::AcornBbc},SystemRule{Formats::AcornDfsPrefix,DiskSystemIds::AcornBbc},
 SystemRule{Formats::EpsonQx10Prefix,DiskSystemIds::EpsonQx10},SystemRule{Formats::MsxPrefix,DiskSystemIds::Msx},SystemRule{DecPrefix,DiskSystemIds::Dec},SystemRule{Formats::UcsdPrefix,DiskSystemIds::Ucsd}};
}
