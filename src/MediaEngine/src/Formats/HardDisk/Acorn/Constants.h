#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AcornWinchester::Constants {
inline constexpr std::int32_t SectorSize=256,SectorsPerTrack=33,MediumSectorSize=512,LargeSectorSize=1024,SmallSequentialSectors=32,MediumSequentialSectors=17,MediumInterleavedSectors=18,LargeSectors=9,MaximumCylinders=65535,MaximumHeads=255;
inline constexpr std::size_t DescriptorSize=33,MinimumDescriptorSize=22,BlockSizeOffset=9,BlockSizeShift=8,CylinderOffset=13,HeadOffset=15,ReducedWriteCurrentOffset=16,LandingZoneOffset=20,StepRateOffset=21,ExtentLengthOffset=3,DriveListOffset=12,SignatureMinimumLength=0x700,HeaderSignatureOffset=0x201,FooterSignatureOffset=0x6fb,Base64Quantum=4;
inline constexpr std::uint8_t ExtentLength=8,DriveListCode=1;
inline constexpr std::string_view FormatId="hdd.acorn-winchester",WriterId="acorn-winchester",Extension=".dat",DescriptorExtension=".dsc",DescriptorKey="acorn.dsc",LogicalBlockSizeKey="logicalBlockSize",Signature="Hugo",Base64Whitespace=" \t\r\n",TranslationDomain="MediaEngine";
inline constexpr std::string_view MissingSectorsMessage="MediaEngine.AcornDatPartial",UnknownGeometryMessage="MediaEngine.AcornDscUnknown";
}
