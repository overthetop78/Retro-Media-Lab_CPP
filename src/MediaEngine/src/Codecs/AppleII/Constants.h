#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::AppleII::Constants {
inline constexpr std::uint8_t FourAndFourMask=0xaa,BitOneMask=2,BitTwoMask=4,LowTwoMask=3,LowThreeMask=7;
inline constexpr std::int32_t SectorSize=256,EncodedAddressByteCount=8,EncodedBytesPerAddressValue=2,SixAndTwoEncodedByteCount=343,SixAndTwoDecodedByteCount=342,SixAndTwoAuxiliaryByteCount=86,FiveAndThreeEncodedByteCount=411,FiveAndThreeAuxiliaryByteCount=154,FiveAndThreeChunkByteCount=51,MinimumSixAndTwoWorkBufferByteCount=258;
inline constexpr int TwoBitShift=2,ThreeBitShift=3,FourBitShift=4,FiveBitShift=5,FiveGroupCount=5,ThreeGroupCount=3,InverseTableSize=256;
inline constexpr std::array<std::uint8_t,64> SixAndTwoTable{0x96,0x97,0x9a,0x9b,0x9d,0x9e,0x9f,0xa6,0xa7,0xab,0xac,0xad,0xae,0xaf,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,0xcb,0xcd,0xce,0xcf,0xd3,0xd6,0xd7,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,0xe5,0xe6,0xe7,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff};
inline constexpr std::array<std::uint8_t,32> FiveAndThreeTable{0xab,0xad,0xae,0xaf,0xb5,0xb6,0xb7,0xba,0xbb,0xbd,0xbe,0xbf,0xd6,0xd7,0xda,0xdb,0xdd,0xde,0xdf,0xea,0xeb,0xed,0xee,0xef,0xf5,0xf6,0xf7,0xfa,0xfb,0xfd,0xfe,0xff};
inline constexpr std::string_view Id="apple2.gcr",DisplayName="Apple II GCR",VolumeAttribute="volume",SectorsPerTrackAttribute="sectorsPerTrack",VolumeField="volumeValue",SectorNumberField="Number";
inline constexpr std::int32_t FiveAndThreeSectorsPerTrack=13,SixAndTwoSectorsPerTrack=16,DefaultVolume=254,MaximumAddressValue=255,LeadingGapBitCount=100,TrailingGapBitCount=32,PrologueBitCount=24,PrologueAdvanceBitCount=23,EncodedAddressBitCount=64,AddressBlockBitCount=88,DataSearchBitCount=1024,CircularTailBitCount=4096,VolumeOffset=0,CylinderOffset=1,SectorOffset=2,ChecksumOffset=3,AddressValueCount=4,ConfidenceSectorWeight=2,BitsPerByte=8;
inline constexpr double ConfidenceDivisor=32;
inline constexpr std::uint8_t SectorSizeCode=1,LogicalHead=0;
inline constexpr std::uint32_t SixAndTwoAddressPrologue=0xd5aa96,FiveAndThreeAddressPrologue=0xd5aab5,DataPrologue=0xd5aaad;
inline constexpr std::array<std::uint8_t,3> SixAndTwoAddressPrologueBytes{0xd5,0xaa,0x96},FiveAndThreeAddressPrologueBytes{0xd5,0xaa,0xb5},EpilogueBytes{0xde,0xaa,0xeb};
inline constexpr std::array<std::uint8_t,9> AddressToDataSeparatorBytes{0xde,0xaa,0xeb,0xff,0xff,0xff,0xd5,0xaa,0xad};
inline constexpr std::string_view AddressKey="MediaEngine.AppleIIAddress",AddressThirteenKey="MediaEngine.AppleIIAddressThirteen",DataKey="MediaEngine.AppleIIData",DataThirteenKey="MediaEngine.AppleIIDataThirteen",TruncatedKey="MediaEngine.AppleIITruncated",UnpairedKey="MediaEngine.AppleIIUnpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
}
