#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaFileSystems::AtariMetadata::KFileConstants {
inline constexpr std::size_t BootSectorCount=3,SectorSize=128,FlagOffset=0,CountOffset=1,LoadOffset=2,JumpOffset=6,JumpLowOffset=7,JumpHighOffset=8,PayloadOffset=9,MinimumExecutableLength=2;
inline constexpr std::uint16_t BootLoad=0x0700,Executable=0xffff;
inline constexpr std::uint8_t Jump=0x4c,JumpLow=0x14,JumpHigh=0x07;
inline constexpr unsigned MiddleShift=8,HighShift=16;
inline constexpr std::string_view Id="atari-k-file",Display="K-file (KBoot)",Bootable="bootable",SingleProgram="single-program",KBoot="kboot";
}
