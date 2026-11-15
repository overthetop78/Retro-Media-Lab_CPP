#pragma once
#include "../Nib/Constants.h"
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
namespace RetroMediaLab::MediaEngine::Woz::Constants {
inline constexpr std::string_view Version1Signature="WOZ1",Version2Signature="WOZ2",InfoId="INFO",TrackMapId="TMAP",TracksId="TRKS",Creator="Retro Media Lab";
inline constexpr std::array<std::uint8_t,4> Marker{0xff,0x0a,0x0d,0x0a};
inline constexpr std::size_t MinimumFileLength=256,SignatureLength=4,MarkerOffset=4,CrcOffset=8,CrcLength=4,ChunksOffset=12;
inline constexpr std::size_t ChunkIdLength=4,ChunkLengthOffset=4,ChunkLengthSize=4,ChunkHeaderLength=8,MinimumInfoLength=2,InfoLength=60;
inline constexpr std::size_t InfoVersionOffset=0,InfoDiskTypeOffset=1,InfoProtectionOffset=2,InfoSynchronizedOffset=3,InfoCleanedOffset=4,InfoCreatorOffset=5;
inline constexpr std::size_t TrackCount=40,MapEntriesPerTrack=4,MapLength=TrackCount*MapEntriesPerTrack,Woz1TrackEntryLength=Nib::Constants::TrackLengthBytes,Woz1BitCountOffset=6648,Woz1BitCountLength=2;
inline constexpr std::size_t BitsPerByte=Nib::Constants::BitsPerByte,Woz1MaximumBitCount=Woz1BitCountOffset*BitsPerByte;
inline constexpr std::size_t Woz2BlockLength=512,Woz2DescriptorLength=8,Woz2StartBlockOffset=0,Woz2BlockCountOffset=2,Woz2BitCountOffset=4,Woz2BlockFieldLength=2,Woz2BitCountLength=4;
inline constexpr std::uint8_t AppleDiskType=1,InfoVersion=1,Writable=0,Synchronized=1,Cleaned=1,MissingDescriptor=0xff,FillByte=0xff;
inline constexpr std::uint32_t CrcPolynomial=0xedb88320;
inline constexpr auto MaximumLength=std::numeric_limits<std::int32_t>::max();
inline constexpr std::uint8_t AsciiMaximum=127,AsciiReplacement='?';
}
