#pragma once
#include <RetroMediaLab/MediaFileSystems/AtariDosConstants.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <array>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::AtariDos::Constants {
using namespace LayoutConstants;
inline constexpr std::string_view Id="atari-dos",MyDosId="atari-mydos",MyDosAttribute="mydos",ExtendedLinksAttribute="extended-sector-links",AtariDosAttribute="atari-dos",RelocatedAttribute="relocated-directory";
inline constexpr std::array<std::string_view,9> Formats{RetroMediaLab::MediaFileSystems::Formats::Atari90,RetroMediaLab::MediaFileSystems::Formats::Atari130,RetroMediaLab::MediaFileSystems::Formats::Atari140,RetroMediaLab::MediaFileSystems::Formats::Atari180,RetroMediaLab::MediaFileSystems::Formats::AtariAtx,RetroMediaLab::MediaFileSystems::Formats::AtariXfd90,RetroMediaLab::MediaFileSystems::Formats::AtariXfd130,RetroMediaLab::MediaFileSystems::Formats::AtariXfd140,RetroMediaLab::MediaFileSystems::Formats::AtariXfd180};
inline constexpr std::int32_t MaximumEntries=64,EntryNumberMask=0x3f,BitsPerByte=8,LinkLowOffset=1,LinkUsedOffset=2,BootSectorCount=3,ExtendedCodeBias=2,ExtendedSingleDensityMultiplier=2,MinimumRelocatedEntries=2,RelocatedMajorityMultiplier=2;
inline constexpr std::uint8_t KnownFlags=InUseFlag|OpenForOutputFlag|CreatedByDos2Flag|DoubleDensityFlag|LockedFlag|DeletedFlag,AsciiMaximum=0x7f,AsciiReplacement='?';
inline constexpr char Dot='.';
inline constexpr std::size_t MaximumBuffer=std::size_t(std::numeric_limits<std::int32_t>::max());
}
