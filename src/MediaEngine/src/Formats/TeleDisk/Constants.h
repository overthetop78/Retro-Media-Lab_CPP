#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::TeleDisk::Constants {
inline constexpr std::size_t HeaderSize=12;
inline constexpr std::size_t SignatureOffset=0;
inline constexpr std::size_t SequenceOffset=2;
inline constexpr std::size_t CheckSignatureOffset=3;
inline constexpr std::size_t ByteFieldSize=1;
inline constexpr std::size_t VersionOffset=4;
inline constexpr std::size_t DataRateOffset=5;
inline constexpr std::size_t DriveTypeOffset=6;
inline constexpr std::size_t SteppingOffset=7;
inline constexpr std::size_t DosModeOffset=8;
inline constexpr std::size_t SurfaceCountOffset=9;
inline constexpr std::size_t HeaderCrcOffset=10;
inline constexpr std::size_t CommentHeaderSize=10;
inline constexpr std::size_t CommentLengthOffset=2;
inline constexpr std::size_t CommentYearOffset=4;
inline constexpr std::size_t CommentMonthOffset=5;
inline constexpr std::size_t CommentDayOffset=6;
inline constexpr std::size_t CommentHourOffset=7;
inline constexpr std::size_t CommentMinuteOffset=8;
inline constexpr std::size_t CommentSecondOffset=9;
inline constexpr std::size_t CommentCrcOffset=0;
inline constexpr std::size_t TrackHeaderSize=4;
inline constexpr std::size_t TrackSectorCountOffset=0;
inline constexpr std::size_t TrackCylinderOffset=1;
inline constexpr std::size_t TrackHeadOffset=2;
inline constexpr std::size_t TrackCrcOffset=3;
inline constexpr std::size_t SectorHeaderSize=6;
inline constexpr std::size_t SectorCylinderOffset=0;
inline constexpr std::size_t SectorHeadOffset=1;
inline constexpr std::size_t SectorNumberOffset=2;
inline constexpr std::size_t SectorSizeCodeOffset=3;
inline constexpr std::size_t SectorFlagsOffset=4;
inline constexpr std::size_t SectorCrcOffset=5;
inline constexpr std::size_t SectorDataHeaderSize=3;
inline constexpr std::size_t EncodedLengthOffset=0;
inline constexpr std::size_t EncodingOffset=2;
inline constexpr std::size_t EncodingFieldSize=1;
inline constexpr std::size_t WordSize=2;
inline constexpr std::size_t BaseSectorSize=128;
inline constexpr std::size_t MaximumSectorSizeCode=6;
inline constexpr std::size_t HeadMask=0x01;
inline constexpr std::size_t CommentPresentMask=0x80;
inline constexpr std::size_t EndOfTracks=0xFF;
inline constexpr std::size_t RepeatedSectorPayloadSize=4;
inline constexpr std::size_t RepeatedSectorCountOffset=0;
inline constexpr std::size_t RepeatedSectorPatternOffset=2;
inline constexpr std::size_t RepeatedSectorSecondPatternByteOffset=3;
inline constexpr std::size_t RleControlSize=2;
inline constexpr std::size_t PatternWordSize=2;
inline constexpr std::uint16_t CrcPolynomial=0xa097,CrcInitial=0;
inline constexpr std::uint8_t DefaultVersion=21,DefaultDataRate=0;
inline constexpr std::uint8_t DataCrcError=0x02,DeletedData=0x04,DataUnavailable=0x10,DataSkipped=0x20,UnavailableMask=DataUnavailable|DataSkipped;
inline constexpr std::string_view Signature="TD",AdvancedSignature="td",Extension=".td0";
inline constexpr std::size_t FirstPatternByte=0,SecondPatternByte=1,BitsPerByte=8;
inline constexpr int WindowSize=4096,LookAhead=60,Threshold=2,ByteSymbols=256,SymbolCount=ByteSymbols+LookAhead-Threshold,MaximumOutputSize=3000000,MaximumFrequency=0x8000,NodeCount=2*SymbolCount-1,Root=NodeCount-1,DictionaryMask=WindowSize-1,PositionLowBits=6,PositionLowMask=0x3f,PositionPrefixAdjustment=2;
inline constexpr int TreeArity=2;
inline constexpr std::uint8_t DictionaryFill=' ';
inline constexpr std::uint16_t FrequencySentinel=0xffff;
inline constexpr std::array<std::uint8_t,64> PositionLengths={3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
inline constexpr std::array<std::uint8_t,64> PositionCodes={
                0x00,
                0x20,
                0x30,
                0x40,
                0x50,
                0x58,
                0x60,
                0x68,
                0x70,
                0x78,
                0x80,
                0x88,
                0x90,
                0x94,
                0x98,
                0x9c,
                0xa0,
                0xa4,
                0xa8,
                0xac,
                0xb0,
                0xb4,
                0xb8,
                0xbc,
                0xc0,
                0xc2,
                0xc4,
                0xc6,
                0xc8,
                0xca,
                0xcc,
                0xce,
                0xd0,
                0xd2,
                0xd4,
                0xd6,
                0xd8,
                0xda,
                0xdc,
                0xde,
                0xe0,
                0xe2,
                0xe4,
                0xe6,
                0xe8,
                0xea,
                0xec,
                0xee,
                0xf0,
                0xf1,
                0xf2,
                0xf3,
                0xf4,
                0xf5,
                0xf6,
                0xf7,
                0xf8,
                0xf9,
                0xfa,
                0xfb,
                0xfc,
                0xfd,
                0xfe,
                0xff
            };
inline constexpr int HexBase=16,WordHexWidth=4,ByteHexWidth=2;
inline constexpr char HexPadding='0';
inline constexpr std::uint8_t HighDensityDataRate=2,FmDataRate=0x80,Drive525Double=1,Drive525High=2,Drive35Double=3,Drive35High=4;
inline constexpr std::uint32_t LowCylinderLimit=40,LowHeadLimit=2;
inline constexpr std::uint64_t DoubleDensityCapacity=737280,HighDensity525Capacity=1228800;
}
