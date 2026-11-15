#pragma once
#include <array>
#include "../Crc16Constants.h"
#include <cstddef>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::DecRx02::Constants {
inline constexpr std::string_view Id="dec.rx02",DisplayName="DEC RX02 M²FM",StructureName="DEC RX02",FmName="FM",M2FmName="M²FM";
inline constexpr std::uint8_t HeaderAddressMark=0xfe,FmDeletedDataMark=0xf8,M2FmDataMark=0xf9,AlternateFaMark=0xfa,FmDataMark=0xfb,AlternateFcMark=0xfc,M2FmDeletedDataMark=0xfd,FmSectorSizeCode=0,M2FmSectorSizeCode=1;
inline constexpr std::int32_t MarkBitCount=32,HeaderDecodedByteCount=6,HeaderCylinderOffset=0,HeaderHeadOffset=1,HeaderSectorOffset=2,HeaderSizeCodeOffset=3,HeaderCrcHighOffset=4,HeaderCrcLowOffset=5,CrcByteCount=2,FmSectorSize=128,M2FmSectorSize=256,EncodedMfmByteBitCount=16,EncodedFmByteBitCount=32,PhaseBitCount=1,HeaderBitCount=224,GapBitCount=64,MaximumAddress=255,MaximumDataSearchDistanceBits=1664,ConfidenceSectorWeight=2,BitsPerByte=8,CellsPerBit=2,EncodeStart=1,EncodeStep=2,EncodeSkip=8,DecodeStart=0,DecodeStep=1,DecodeSkip=9;
inline constexpr std::uint64_t MaximumBits=2147483647,MaximumDecodedBytes=(MaximumBits-PhaseBitCount)/EncodedMfmByteBitCount;
inline constexpr std::size_t FmNormalIndex=3,FmDeletedIndex=0,M2FmNormalIndex=1,M2FmDeletedIndex=5;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,11> NormalRule{0,0,1,0,1,0,1,0,1,0,0},EncodedRule{0,1,0,0,0,1,0,0,0,1,0};
inline constexpr std::string_view HeaderKey="MediaEngine.Rx02Header",HeaderNoMarkKey="MediaEngine.Rx02HeaderNoMark",DataKey="MediaEngine.Rx02Data",TruncatedKey="MediaEngine.Rx02Truncated",UnpairedKey="MediaEngine.Rx02Unpaired",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable",HexDigits="0123456789ABCDEF";
inline constexpr int BitsPerNibble=4,NibbleMask=15;
inline constexpr std::uint16_t CrcPolynomial=Crc16::Constants::Ccitt,CrcInitialValue=Crc16::Constants::AllBitsSet;

}
