#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Adaptec::Constants {
inline constexpr std::size_t ParameterFlagCount=9,ParameterCylinderOffset=2,ParameterHeadUpperOffset=4,ParameterDerivedOffset=6,ParameterLandingOffset=8;
inline constexpr int ParameterWordShift=7,ParameterAuxiliaryShift=5,ParameterHeadUpperShift=2,MaximumCylinders=2048,MaximumHeads=16,MinimumBlockSize=256,MediumBlockSize=512,MaximumBlockSize=1024,MaximumReducedCurrentCylinder=2047;
inline constexpr std::uint8_t DefectCountMask=0x7f,ParameterWordHighMask=0x1f,ParameterAuxiliaryMask=3,LandingBeforeTrackZeroMask=0x80,LandingSignFlag=0x40,MaximumStepCode=2;
inline constexpr std::string_view EncodingId="adaptec.mfm";
inline constexpr std::uint32_t CellNanoseconds=100,AddressMask=0x7fffff,DefectiveMask=0x800000,CheckPolynomial=0x41044185,CheckHighBit=0x80000000,NominalIndexNanoseconds=16666667;
inline constexpr std::int32_t CellsPerByte=16,BitsPerByte=8,MinimumSyncBits=64,CellsPerBit=2,CheckHighByteShift=24,DefaultInterleave=2;
inline constexpr std::uint16_t SyncWord=0x4489;
inline constexpr std::uint8_t SyncByte=0xa1,IdMark=0xfe,DataMark=0xf8,DefectiveAddressByteMask=0x80,LastSectorFlag=0x80,GapByte=0x4e,InvalidCheckMask=1;
inline constexpr std::size_t IdLength=10,IdContentLength=6,AddressOffset=2,AddressLength=3,FlagsOffset=5,DataOffset=2,CheckLength=4,DataOverhead=DataOffset+CheckLength;
inline constexpr std::int32_t MaximumSectors=33,FirstGapLength=9,SequentialGapLength=19,IdPreambleLength=12,DataPreambleLength=15,PostDataLength=2,InterleavedRecordOverhead=54,SequentialRecordOverhead=64;
inline constexpr std::int32_t SmallSequentialSectors=32,SmallInterleavedSectors=33,MediumSequentialSectors=17,MediumInterleavedSectors=18,LargeSectors=9;
inline constexpr std::array<std::int32_t,3> CandidateSizes{MinimumBlockSize,MediumBlockSize,MaximumBlockSize};
}
