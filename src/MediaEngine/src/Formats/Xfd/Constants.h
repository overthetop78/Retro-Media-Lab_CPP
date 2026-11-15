#pragma once
#include "../Atari8Bit/Constants.h"
#include <array>
namespace RetroMediaLab::MediaEngine::Xfd::Constants {
using namespace Atari8Bit::Constants;
inline constexpr std::string_view Extension=".xfd";
inline constexpr std::uint32_t PaddingLength=16;
inline constexpr std::array<Profile,4> Profiles{{{AtariXfd90,SingleSize,StandardCount},{AtariXfd130,SingleSize,EnhancedCount},{AtariXfd140,SingleSize,ExtendedCount},{AtariXfd180,DoubleSize,StandardCount}}};
}
