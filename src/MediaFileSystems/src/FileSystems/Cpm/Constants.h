#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <RetroMediaLab/MediaFileSystems/CpmLayoutConstants.h>
#include <array>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Cpm::Constants {
inline constexpr std::string_view Id="cpm",AmstradId="amstrad.cpm",UserPrefix="CP/M user ",UserAreaKey="MediaFileSystems.CpmUserArea",TranslationDomain="MediaFileSystems";
inline constexpr std::array<std::string_view,8> GenericFormats{Formats::Commodore1541,Formats::Commodore1571,Formats::Commodore1581,Formats::EpsonQx10_320,Formats::EpsonQx10_396,Formats::EpsonQx10_399,Formats::EpsonQx10_400,Formats::EpsonQx10Logo};
inline constexpr std::array<std::string_view,3> AmstradFormats{Formats::AmstradCpc,Formats::AmstradPcw,Formats::CpcEmuDsk};
inline constexpr std::array<std::array<std::int32_t,6>,8> GenericLayouts{{{0x0a00,0x0a00,64,1024,2,0},{0x0a00,0x0a00,128,2048,2,1},{0,0,128,2048,2,1},{4*2*16*256,4*2*16*256,64,2048,2,0},{4*16*256,4*16*256,64,2048,2,0},{16*256,16*256,64,2048,2,0},{2*2*5*1024,2*2*5*1024,64,2048,2,0},{4*16*256,4*16*256,64,2048,2,0}}};
inline constexpr std::array<std::int32_t,3> SupportedSectorSizes{256,512,1024};
inline constexpr std::int32_t DirectoryEntrySize=32,RecordSize=128,FileNameOffset=1,FileNameLength=8,ExtensionOffset=9,ExtensionLength=3,ExtentLowOffset=12,ExtentHighOffset=14,RecordCountOffset=15,AllocationOffset=16,ExtentHighShift=5,NarrowAllocationCount=16,WideAllocationCount=8,WideAllocationSize=2,NoStorageReference=-1,MinimumDirectoryScore=4,MaximumEpsonDirectorySearch=64*1024;
inline constexpr std::uint8_t UnusedEntry=0xe5,VolumeLabelUser=0x20,PasswordLabelUser=0x21,MaximumUser=31,AttributeMask=0x7f,FirstPrintable=0x21,LastPrintable=0x7e,LowercaseFirst='a',LowercaseLast='z',UppercaseFirst='A',UppercaseLast='Z',DigitFirst='0',DigitLast='9';
inline constexpr char Space=' ',Dash='-',Underscore='_',Dot='.';
inline constexpr std::int32_t CpcSystemFirst=AmstradLayoutConstants::SystemFirstSectorId,CpcSystemLast=AmstradLayoutConstants::SystemLastSectorId,CpcDataFirst=AmstradLayoutConstants::DataFirstSectorId,CpcDataLast=AmstradLayoutConstants::DataLastSectorId,CpcDirectoryEntries=AmstradLayoutConstants::CpcDirectoryEntries,CpcAllocationSize=AmstradLayoutConstants::CpcAllocationSize,CpcDirectoryBlocks=AmstradLayoutConstants::CpcDirectoryBlocks,CpcSystemReserved=AmstradLayoutConstants::CpcSystemReservedTracks,CpcSectorsPerTrack=AmstradLayoutConstants::CpcSectorsPerTrack,CpcSectorSize=AmstradLayoutConstants::CpcSectorSize;
inline constexpr std::int32_t SpecificationMinimumLength=AmstradSpecificationConstants::MinimumLength,TracksOffset=AmstradSpecificationConstants::TracksOffset,SectorsPerTrackOffset=AmstradSpecificationConstants::SectorsPerTrackOffset,SectorSizeCodeOffset=AmstradSpecificationConstants::SectorSizeCodeOffset,ReservedTracksOffset=AmstradSpecificationConstants::ReservedTracksOffset,AllocationSizeCodeOffset=AmstradSpecificationConstants::AllocationSizeCodeOffset,DirectoryBlocksOffset=AmstradSpecificationConstants::DirectoryBlocksOffset,MaximumTracks=AmstradSpecificationConstants::MaximumTracks,MaximumSectorsPerTrack=AmstradSpecificationConstants::MaximumSectorsPerTrack,MaximumReservedTracks=AmstradSpecificationConstants::MaximumReservedTracks,MaximumDirectoryBlocks=AmstradSpecificationConstants::MaximumDirectoryBlocks,MinimumSectorSize=AmstradSpecificationConstants::MinimumSectorSize,MaximumSectorSize=AmstradSpecificationConstants::MaximumSectorSize,MinimumAllocationSize=AmstradSpecificationConstants::MinimumAllocationSize,MaximumAllocationSize=AmstradSpecificationConstants::MaximumAllocationSize;
inline constexpr std::uint8_t SizeCodeMask=AmstradSpecificationConstants::SizeCodeMask;
inline constexpr std::size_t MaximumBuffer=std::size_t(std::numeric_limits<std::int32_t>::max());
inline constexpr std::int32_t MajorityMultiplier=2;
}
