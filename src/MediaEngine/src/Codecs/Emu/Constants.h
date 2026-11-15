#pragma once
#include <array>
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::EmuFm::Constants {
inline constexpr std::string_view Id="emu.fm",DisplayName="E-mu Emulator FM",HeaderKey="MediaEngine.EmuHeader",DataKey="MediaEngine.EmuData",UnknownKey="MediaEngine.EmuUnclassified",ValidKey="MediaEngine.IntegrityValid",InvalidKey="MediaEngine.IntegrityInvalid",UnavailableKey="MediaEngine.IntegrityUnavailable";
inline constexpr std::int32_t MarkBitCount=64,EncodedByteBitCount=32,HeaderDecodedByteCount=3,HeaderBitCount=160,SectorSize=3584,DataBlockByteCount=3586,TrackShift=1,MaximumCylinder=127,MaximumHead=1,GapBitCount=64,MaximumDataSearchDistanceBits=1664,ConfidenceSectorWeight=2,BitsPerByte=8;
inline constexpr std::uint8_t SectorNumber=1,SectorSizeCode=0,HeadMask=1;
inline constexpr double ConfidenceDivisor=20;
inline constexpr std::array<std::uint8_t,8> SectorMark{0x45,0x45,0x55,0x55,0x45,0x54,0x54,0x45};
}
