#include "Mfm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AmigaMfm {
namespace {
void validate(MediaFileSystems::Bytes encoded,std::int64_t offset,std::int64_t count){
    if(offset<0||count<0||std::uint64_t(offset)>encoded.size()||std::uint64_t(count)>encoded.size()-std::uint64_t(offset)||count%Constants::InfoByteCount)
        throw MediaError(DiagnosticCode::AmigaMfmParityRange,{std::to_string(offset),std::to_string(count),std::to_string(encoded.size())});
}
}
Parity parity(MediaFileSystems::Bytes encoded,std::int64_t offset,std::int64_t count,std::stop_token token){
    checkCancellation(token);validate(encoded,offset,count);std::uint8_t high=0,low=0;
    for(std::int64_t index=0;index<count;index+=Constants::InfoByteCount){
        checkCancellation(token);const auto start=std::size_t(offset+index);
        high^=std::uint8_t(encoded[start]^encoded[start+Constants::BitStride]);
        low^=std::uint8_t(encoded[start+1]^encoded[start+Constants::BitStride+1]);
    }
    return {high,low};
}
Parity splitParity(MediaFileSystems::Bytes encoded,std::int64_t offset,std::int64_t count,std::stop_token token){
    checkCancellation(token);validate(encoded,offset,count);std::uint8_t high=0,low=0;const auto half=count/Constants::BitStride;
    for(std::int64_t index=0;index<half;index+=Constants::BitStride){
        checkCancellation(token);const auto start=std::size_t(offset+index),other=std::size_t(offset+half+index);
        high^=std::uint8_t(encoded[start]^encoded[other]);low^=std::uint8_t(encoded[start+1]^encoded[other+1]);
    }
    return {high,low};
}
}
