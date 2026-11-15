#pragma once
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Crc16::Constants {
inline constexpr std::uint16_t Ccitt=0x1021,Ibm=0x8005,AllBitsSet=0xffff,Zero=0,HighBit=0x8000;
inline constexpr int ByteBits=8,CrcBytes=2;
}
