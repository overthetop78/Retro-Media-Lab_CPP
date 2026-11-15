#include "BitLatch.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
namespace RetroMediaLab::MediaEngine::Apple {
std::optional<std::vector<std::uint8_t>> readBytes(Flux::BitView bits,std::int64_t& offset,std::int64_t count,std::stop_token token){
    checkCancellation(token);
    if(count<0)throw MediaError(DiagnosticCode::AppleLatchCount,{std::to_string(count)});
    if(offset<0||std::uint64_t(offset)>bits.size())throw MediaError(DiagnosticCode::AppleLatchOffset,{std::to_string(offset),std::to_string(bits.size())});
    if(count>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::FluxSizeLimit);
    std::vector<std::uint8_t> result(static_cast<std::size_t>(count));
    for(auto& value:result){
        checkCancellation(token);
        if(bits.size()-std::size_t(offset)<Constants::BitsPerByte)return std::nullopt;
        for(int bit=0;bit<Constants::BitsPerByte;++bit)value=std::uint8_t((value<<1)|(bits[std::size_t(offset++)]?1:0));
        while((value&Constants::SynchronizedByteMask)==0){
            checkCancellation(token);if(std::uint64_t(offset)>=bits.size())return std::nullopt;
            value=std::uint8_t((value<<1)|(bits[std::size_t(offset++)]?1:0));
        }
    }
    return result;
}
}
