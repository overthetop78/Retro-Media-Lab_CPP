#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::AtariDos::LayoutConstants {
inline constexpr std::int32_t VtocSector=360,FirstDirectorySector=361,DirectorySectorCount=8,LastDirectorySector=FirstDirectorySector+DirectorySectorCount-1,MinimumSectorSize=128,DirectoryEntrySize=16,DirectoryEntriesPerSector=8;
inline constexpr std::int32_t FlagsOffset=0,SectorCountOffset=1,FirstSectorOffset=3,NameOffset=5,NameLength=8,ExtensionLength=3,LinkByteCount=3,FileOwnerShift=2,FreeSectorCountOffset=3,UsableSectorCountOffset=1,FreeSectorCountLength=2;
inline constexpr std::uint8_t InUseFlag=0x40,OpenForOutputFlag=0x01,CreatedByDos2Flag=0x02,DoubleDensityFlag=0x04,LockedFlag=0x20,DeletedFlag=0x80,NextSectorHighMask=3,VtocMarker=2,DoubleDensityVtocMarker=3,LegacyVtocMarker=0,MinimumExtendedVtocCode=4,NamePadding=0x20,MinimumNameCharacter=0x20,MaximumNameCharacter=0x7e;
inline constexpr std::string_view BoundedSectorViewAttribute="bounded-sector-view",SharedSectorViewAttribute="shared-sector-view";
}
