#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Scp::Constants {
inline constexpr std::array<std::uint8_t,3> Signature{'S','C','P'}, TrackSignature{'T','R','K'};
inline constexpr std::uint32_t HeaderBytes=16,TrackSlots=168,WordBytes=2,FieldBytes=4,TrackHeaderBytes=4,RevolutionDescriptorBytes=12;
inline constexpr std::uint32_t VersionOffset=3,DiskTypeOffset=4,RevolutionsOffset=5,StartOffset=6,EndOffset=7,FlagsOffset=8,WidthOffset=9,HeadsOffset=10,ResolutionOffset=11,ChecksumOffset=12;
inline constexpr std::uint32_t CountOffset=4,DataOffset=8,TrackNumberOffset=3,TableBytes=TrackSlots*FieldBytes,FullHeaderBytes=HeaderBytes+TableBytes;
inline constexpr std::uint32_t MinimumRevolutions=1,MaximumRevolutions=64,ExplicitWidth=16,MaximumHeads=2,HeadCount=2;
inline constexpr std::uint32_t WritableFlag=0x10,ExtendedFlag=0x40,OverflowTicks=65536,ChunkBytes=65536;
inline constexpr std::uint64_t MaximumLength=2147483647,MaximumInterval=4294967295;
inline constexpr std::int32_t RoundingDivisor=2,ResolutionStep=25,VersionShift=4,VersionMask=15;
inline constexpr double NanosecondsPerMillisecond=1000000,MillisecondsPerMinute=60000;
inline constexpr std::uint8_t CaptureVersion=0x19,CaptureResolution=0,CaptureRevolutions=1;
inline constexpr std::uint64_t InitialFileSize=0;
inline constexpr std::uint8_t VisualizationVersion=0,VisualizationDiskType=0,DefaultWidth=0;
inline constexpr std::uint8_t IndexAlignedFlag=0x01,Tpi96Flag=0x02,Rpm360Flag=0x04,NormalizedFlag=0x08,ThirdPartyFlag=0x80;
inline constexpr std::uint8_t VisualizationFlags=IndexAlignedFlag|WritableFlag,ConversionFlags=IndexAlignedFlag|NormalizedFlag|ThirdPartyFlag;
inline constexpr std::uint8_t BothHeads=0,Side0=1,Side1=2;
inline constexpr std::uint32_t MissingChecksum=0;
inline constexpr std::int32_t LowDensityLastCylinder=41;
inline constexpr std::string_view VersionSeparator=".";
inline constexpr std::string_view RawVersionKey="scp.version.raw",DiskTypeKey="scp.diskType",FlagsKey="scp.flags",WidthKey="scp.bitCellEncoding",ResolutionKey="scp.resolution";
inline constexpr std::uint8_t FooterFlag=0x20,NoFlags=0;
inline constexpr std::uint32_t ByteMaximum=255,MinimumTick=1,DecimalBase=10;
inline constexpr std::uint8_t RetainedFlags=ByteMaximum^(FooterFlag|ExtendedFlag);
inline constexpr char FirstDigit='0',LastDigit='9';
}
