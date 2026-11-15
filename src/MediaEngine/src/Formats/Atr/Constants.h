#pragma once
#include "../Atari8Bit/Constants.h"
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Atr::Constants {
using namespace Atari8Bit::Constants;
inline constexpr std::uint16_t Signature=0x0296;
inline constexpr std::size_t HeaderSize=16,SignatureOffset=0,ParagraphLow=2,SectorSizeOffset=4,ParagraphHigh=6;
inline constexpr std::uint32_t ParagraphSize=16,HighWordShift=16,BitsPerByte=8;
inline constexpr std::uint32_t VtocSector=360,EnhancedUsable=1010,XfManaged=1027,VtocDos2=2,VtocDos3=3;
inline constexpr std::uint8_t DirectoryMask=0x40,ZeroPadding=0,ExternalPadding=0x1a;
inline constexpr std::size_t BootCountOffset=1,ManagedCountOffset=1;
inline constexpr std::int64_t MaxLength=2147483647;
inline constexpr std::string_view UnknownSignature="?",SectorSizes="128, 256, 512";
inline constexpr char HexFill='0';
inline constexpr std::int32_t SignatureHexWidth=4;
inline constexpr std::string_view HexPrefix="0x";
}
