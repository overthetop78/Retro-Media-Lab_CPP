#include "Mfm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AmigaMfm {
namespace {
using namespace Constants;
std::uint8_t interleave(std::uint8_t odd,std::uint8_t even){
    std::uint8_t value=0;
    for(int index=0;index<NibbleBitCount;++index){
        value|=std::uint8_t(((odd>>(NibbleLastBit-index))&1)<<(OddFirstBit-index*BitStride));
        value|=std::uint8_t(((even>>(NibbleLastBit-index))&1)<<(EvenFirstBit-index*BitStride));
    }
    return value;
}
std::uint8_t nibble(std::uint8_t value,bool odd){
    std::uint8_t result=0;const auto first=odd?OddFirstBit:EvenFirstBit;
    for(int index=0;index<NibbleBitCount;++index)result|=std::uint8_t(((value>>(first-index*BitStride))&1)<<(NibbleLastBit-index));
    return result;
}
}
std::vector<std::uint8_t> decodeOddEven(MediaFileSystems::Bytes encoded,std::stop_token token){
    checkCancellation(token);std::vector<std::uint8_t> result(encoded.size());const auto half=encoded.size()/Constants::BitStride;
    for(std::size_t index=0;index<half;++index){
        checkCancellation(token);const auto odd=encoded[index],even=encoded[index+half];
        result[index*Constants::BitStride]=interleave(std::uint8_t(odd>>Constants::NibbleBitCount),std::uint8_t(even>>Constants::NibbleBitCount));
        result[index*Constants::BitStride+1]=interleave(std::uint8_t(odd&Constants::NibbleMask),std::uint8_t(even&Constants::NibbleMask));
    }
    return result;
}
std::vector<std::uint8_t> encodeOddEven(MediaFileSystems::Bytes values,std::stop_token token){
    checkCancellation(token);
    if(values.size()%Constants::BitStride)throw MediaError(DiagnosticCode::AmigaMfmOddCount,{std::to_string(values.size())});
    std::vector<std::uint8_t> result(values.size());const auto half=values.size()/Constants::BitStride;
    for(std::size_t index=0;index<values.size();index+=Constants::BitStride){
        checkCancellation(token);
        result[index/Constants::BitStride]=std::uint8_t((nibble(values[index],true)<<Constants::NibbleBitCount)|nibble(values[index+1],true));
        result[index/Constants::BitStride+half]=std::uint8_t((nibble(values[index],false)<<Constants::NibbleBitCount)|nibble(values[index+1],false));
    }
    return result;
}
}
