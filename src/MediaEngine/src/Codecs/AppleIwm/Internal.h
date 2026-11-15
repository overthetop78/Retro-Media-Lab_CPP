#pragma once
#include "Codec.h"
namespace RetroMediaLab::MediaEngine::AppleIwm::Internal {
struct Accumulator {
 std::uint32_t first=0,second=0,third=0;
 void rotateFirst()noexcept{first=(first&Constants::ChecksumByteMask)<<1;if(first&Constants::ChecksumCarryBit)++first;}
 void addFirst(std::uint8_t value)noexcept{third+=value;if(first&Constants::ChecksumCarryBit){++third;first&=Constants::ChecksumByteMask;}}
 void addSecond(std::uint8_t value)noexcept{second+=value;if(third>Constants::ChecksumByteMask){++second;third&=Constants::ChecksumByteMask;}}
 void addThird(std::uint8_t value)noexcept{first+=value;if(second>Constants::ChecksumByteMask){++first;second&=Constants::ChecksumByteMask;}}
 std::uint8_t packed()const noexcept{return std::uint8_t(((first&Constants::ChecksumHighBitsMask)>>Constants::FirstChecksumShift)|((second&Constants::ChecksumHighBitsMask)>>Constants::SecondChecksumShift)|((third&Constants::ChecksumHighBitsMask)>>Constants::ThirdChecksumShift));}
};
using Group=std::array<std::uint8_t,Constants::GroupByteCount>;
}
