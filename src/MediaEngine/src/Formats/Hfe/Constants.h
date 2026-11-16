#pragma once
#include <cstdint>
#include <string_view>
#include <limits>
namespace RetroMediaLab::MediaEngine::Hfe::Constants {
inline constexpr std::string_view FormatId="raw.hfe",Extension=".hfe";
inline constexpr std::string_view WriterId="hfe";
inline constexpr std::string_view Signature="HXCPICFE",Signature3="HXCHFEV3",RevisionKey="hfe.revision",EncodingKey="hfe.encoding",BitRateKey="hfe.bitRate",ScpDiskTypeKey="scp.diskType";
inline constexpr std::string_view WeakKey="MediaEngine.HfeWeakFeature",IndexKey="MediaEngine.HfeIndexFeature";
inline constexpr std::string_view IsoMfmId="iso.mfm",AmigaMfmId="amiga.mfm",IsoFmId="iso.fm";
inline constexpr std::size_t BlockSize=512,SideChunk=256,SignatureLength=8,RevisionOffset=8,CylinderOffset=9,HeadOffset=10,EncodingOffset=11,BitRateOffset=12,RpmOffset=14,InterfaceOffset=16,ProtectedOffset=17,TableOffset=18,WriteAllowedOffset=20,SingleStepOffset=21,EntrySize=4,LengthOffset=2;
inline constexpr std::uint8_t Revision=0,Nop=0x0f,Random=0x2f,Rate=0x4f,Index=0x8f,Skip=0xcf,NominalDivider=144,OpcodeMask=0x0f,RandomBits=0x55,SevenBitsMask=0x7f;
inline constexpr std::uint8_t IsoMfm=0,AmigaMfm=1,IsoFm=2,Unknown=0xff,WriteProtected=1,WriteAllowed=0xff,WriteForbidden=0,SingleStep=0xff,HeaderPadding=0xff,TrackPadding=0x88;
inline constexpr std::uint32_t MaxHeads=2,TickNanoseconds=25,NanosecondsPerSecond=1000000000,ClockMegahertz=36,NanosecondsPerMicrosecond=1000,NominalBitRate=125,BitsPerDataBit=2,BitsPerByte=8,BitMask=7;
inline constexpr std::size_t MaxSide=32767,MaxCylinder=255,MaxField=65535,MaxLength=2147483647;
inline constexpr std::uint64_t MaxTicks=std::numeric_limits<std::uint32_t>::max();
inline constexpr char FirstDigit='0',LastDigit='9';
inline constexpr std::uint32_t DecimalBase=10;
}
