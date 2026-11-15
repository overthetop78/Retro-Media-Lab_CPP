#include "Crc16.h"
#include "../Models/Flux/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Crc16 {
std::uint16_t update(std::uint16_t crc,std::uint8_t value,std::uint16_t polynomial) noexcept {
    crc^=std::uint16_t(value<<Constants::ByteBits);
    for(int bit=0;bit<Constants::ByteBits;++bit)crc=std::uint16_t(crc&Constants::HighBit?(crc<<1)^polynomial:crc<<1);
    return crc;
}
std::uint16_t compute(MediaFileSystems::Bytes values,std::uint16_t polynomial,std::uint16_t initial,std::stop_token token){
    checkCancellation(token);auto crc=initial;for(auto value:values){checkCancellation(token);crc=update(crc,value,polynomial);}return crc;
}
std::vector<std::uint8_t> append(MediaFileSystems::Bytes values,std::uint16_t polynomial,std::uint16_t initial,std::stop_token token){
    checkCancellation(token);if(values.size()>Flux::Constants::MaximumBits-Constants::CrcBytes)throw MediaError(DiagnosticCode::FluxSizeLimit);
    const auto crc=compute(values,polynomial,initial,token);std::vector<std::uint8_t> result(values.begin(),values.end());
    result.push_back(std::uint8_t(crc>>Constants::ByteBits));result.push_back(std::uint8_t(crc));return result;
}
}
