#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Constants {
inline constexpr std::string_view FileSystemId="commodore-dos",TranslationDomain="MediaFileSystems";
inline constexpr std::int32_t SectorSize=256,LinkLength=2,DirectorySlots=8,DirectoryEntrySize=32,TypeOffset=2,DataTrackOffset=3,DataSectorOffset=4,NameOffset=5,NameLength=16,DeclaredBlocksOffset=30,RecordLengthOffset=23,RecordLengthShift=8,DataBytes=254,MaximumPlausibleChain=64;
inline constexpr std::int32_t HeaderTrack1541=18,HeaderSector1541=0,DirectorySector1541=1,Name1541=0x90,BamOffset1541=4,BamSize1541=4;
inline constexpr std::int32_t HeaderTrack1581=40,HeaderSector1581=0,DirectorySector1581=3,Name1581=4,BamOffset1581=16,BamSize1581=6,BamEntries1581=40;
inline constexpr std::array<std::int32_t,2> BamSectors1581{1,2};
inline constexpr std::uint8_t Signature1541=0x41,Signature1581=0x44,BaseTypeMask=7,Seq=1,Prg=2,Usr=3,Rel=4,Cbm=5,Locked=0x40,Closed=0x80;
inline constexpr std::array<std::string_view,6> TypeNames{"DEL","SEQ","PRG","USR","REL","CBM"};
inline constexpr std::array<std::string_view,4> CommentKeys{"MediaFileSystems.CbmType","MediaFileSystems.CbmTypeOpen","MediaFileSystems.CbmTypeLocked","MediaFileSystems.CbmTypeOpenLocked"};
inline constexpr std::size_t LockedCommentIndex=2;
inline constexpr std::int32_t RelativeSideOffset=21,RelativePointersPerSector=120,MaximumRelativeSides=6,RelativeNumberOffset=2,RelativeRecordOffset=3,RelativeTableOffset=4,RelativePointersOffset=16;
inline constexpr std::int32_t DiskId1541=0xa2,DosType1541=0xa5,DiskId1581=22,DosType1581=25,BamDiskId1581=4,BitsPerByte=8,AddressBytes=2;
inline constexpr std::array<std::uint8_t,2> DiskIdentity{'G','W'},DosIdentity1541{'2','A'},DosIdentity1581{'3','D'};
inline constexpr std::uint8_t DefaultRecordLength=255;
inline constexpr std::string_view RootPath="/";
}
