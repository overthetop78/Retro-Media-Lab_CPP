#pragma once
#include "Sources.h"
#include "BytePrimitiveConstants.h"
namespace RetroMediaLab::MediaFileSystems::BytePrimitives {
inline std::uint8_t reverseBits(std::uint8_t value)noexcept {
    std::uint8_t result=0;
    for(std::size_t bit=0;bit<Constants::BitsPerByte;++bit){result=std::uint8_t((result<<Constants::LeastSignificantBitMask)|(value&Constants::LeastSignificantBitMask));value>>=Constants::LeastSignificantBitMask;}
    return result;
}
inline void writeLittle24(MutableBytes data,std::size_t offset,std::uint32_t value) {
    if(offset>data.size()||Constants::UInt24Bytes>data.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    for(std::size_t i=0;i<Constants::UInt24Bytes;++i)data[offset+i]=std::uint8_t(value>>(i*Constants::BitsPerByte));
}
}
