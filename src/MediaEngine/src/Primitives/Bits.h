#pragma once
#include "BitConstants.h"
#include <cstdint>
namespace RetroMediaLab::MediaEngine::Primitives {
inline std::uint8_t reverseBits(std::uint8_t value)noexcept{std::uint8_t result=0;for(int bit=0;bit<Constants::BitsPerByte;++bit){result=std::uint8_t((result<<1)|(value&1));value>>=1;}return result;}
}
