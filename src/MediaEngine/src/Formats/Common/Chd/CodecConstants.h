#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Chd::CodecConstants {
inline constexpr int ByteBits=8,WordBits=32,RefillLimit=24,HuffmanMaximumBits=24,HuffmanHistogramSize=33,HuffmanLengthMask=31,HuffmanEntryShift=5,HuffmanLongBits=16,HuffmanMediumBits=8,HuffmanLongWidth=5,HuffmanMediumWidth=4,HuffmanShortWidth=3,HuffmanRunBias=3,SmallSymbols=24,SmallMaximumBits=6,SmallLengthBits=3,SmallStop=7,SmallRunBias=2,SmallRunEscape=9;
inline constexpr std::size_t TransferChunk=65536,PollMask=16383,MaximumHunk=128*1024*1024;
inline constexpr std::uint32_t MinimumDictionary=4096,MaximumDictionary=1u<<26,DictionaryRoundingThreshold=1u<<21,DictionaryMiB=1u<<20;
inline constexpr int DictionaryFirstBits=11,DictionaryEven=2,DictionaryOdd=3,LzmaLc=3,LzmaLp=0,LzmaPb=2,ZstdMaximumWindowLog=31;
inline constexpr int FlacMaximumWidth=33,FlacSyncBits=14,FlacSync=0x3ffe,FlacCodeBits=4,FlacSizeCodeBits=3,FlacDefaultSampleBits=16,FlacStereoBytes=4,FlacSampleBytes=2,FlacIndependent=1,FlacLeftSide=8,FlacRightSide=9,FlacMidSide=10,FlacReservedRate=15,FlacFrameNumberHigh=0x80,FlacFrameNumberMinimum=2,FlacFrameNumberMaximum=7,FlacShortBlockBase=192,FlacMiddleBlockBase=576,FlacMiddleBlockFirst=2,FlacMiddleBlockLast=5,FlacByteBlock=6,FlacWordBlock=7,FlacLongBlockFirst=8,FlacLongBlockBase=256,FlacByteRate=12,FlacWordRate1=13,FlacWordRate2=14,FlacSubframeTypeBits=6,FlacFixedFirst=8,FlacFixedLast=12,FlacLpcFirst=32,FlacLpcOrderBias=31,FlacMaximumOrder=32,FlacPrecisionBits=4,FlacReservedPrecision=16,FlacShiftBits=5,FlacResidualMethodBits=2,FlacPartitionBits=4,FlacRiceWidth=4,FlacRice2Width=5,FlacRawWidthBits=5;
inline constexpr std::array<int,8> FlacSampleBits{16,8,12,0,16,20,24,32};
inline constexpr std::array<std::array<int,4>,5> FlacFixedCoefficients{{{{0,0,0,0}},{{1,0,0,0}},{{2,-1,0,0}},{{3,-3,1,0}},{{4,-6,4,-1}}}};
inline constexpr int FlacHeaderHigh=0x80,FlacHeaderPolynomial=7,ByteMask=255;
inline constexpr std::uint16_t FlacFramePolynomial=0x8005;
}
