#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
namespace RetroMediaLab::MediaEngine::KryoFlux::Constants {
inline constexpr std::uint8_t Nop1=8,Nop2=9,Nop3=10,Overflow16=11,Flux3=12,OutOfBand=13;
inline constexpr std::uint8_t StreamInfo=1,Index=2,StreamEnd=3,Info=4,EndOfFile=13,MaximumShortOpcode=7;
inline constexpr std::size_t OutOfBandHeader=4,TypeOffset=1,SizeOffset=2,IndexBytes=4,ShortBytes=2,LongBytes=3,LongLowByteOffset=2;
inline constexpr int ByteBits=8,ResolutionNanoseconds=1,MinimumRecognitionBytes=8,RecognitionHeaderBytes=512;
inline constexpr std::uint64_t OverflowIncrement=0x10000,MaximumDuration=0xffffffff,MaximumTotalLength=0x7fffffffffffffffULL;
inline constexpr double DefaultMasterClock=18432000.0*73.0/14.0/2.0,DefaultSampleClock=DefaultMasterClock/2.0,NanosecondsPerSecond=1000000000.0;
inline constexpr std::int32_t MaximumTrack=99,MaximumHead=1;
inline constexpr std::string_view FormatId="raw.kryoflux",Extension=".raw",FilePattern="^(.*?)(\\d{2})\\.([01])\\.raw$",SampleClockKey="sck";
inline constexpr int PrefixGroup=1,CylinderGroup=2,HeadGroup=3;
inline constexpr char Null='\0',EntrySeparator=',',ValueSeparator='=';
}
