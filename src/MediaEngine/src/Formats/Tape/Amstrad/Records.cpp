#include "Records.h"
#include "Constants.h"
#include "../../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AmstradTape {
using namespace Constants;
std::optional<Record> decodeRecord(std::span<const std::uint8_t> encoded,std::int64_t position,std::stop_token token){
    checkCancellation(token);if(encoded.size()<1+SegmentDataLength+SegmentCrcLength||(encoded.front()!=HeaderSync&&encoded.front()!=DataSync))return {};
    const auto body=encoded.subspan(1);int segments=0;
    for(auto trailer:{TrailerByteCount,AlternateTrailerLength})for(int count=1;count<=(encoded.front()==HeaderSync?1:MaximumSegments);++count){
        const auto used=count*(SegmentDataLength+SegmentCrcLength);if(body.size()!=used+trailer||!std::all_of(body.begin()+used,body.end(),[](auto value){return value==TrailerByte;}))continue;segments=count;
    }
    if(!segments)return {};Record result{encoded.front(),std::vector<std::uint8_t>(segments*SegmentDataLength),true,{position}};
    for(int index=0;index<segments;++index){
        checkCancellation(token);const auto offset=index*(SegmentDataLength+SegmentCrcLength);const auto data=body.subspan(offset,SegmentDataLength);std::copy(data.begin(),data.end(),result.data.begin()+index*SegmentDataLength);
        const auto expected=static_cast<std::uint16_t>(~Crc16::compute(data,CrcPolynomial,CrcSeed,token));result.valid=result.valid&&(qFromBigEndian<std::uint16_t>(body.data()+offset+SegmentDataLength)==expected);
    }
    return result;
}
std::vector<std::uint8_t> encodeRecord(std::uint8_t sync,std::span<const std::uint8_t> data,std::stop_token token){
    checkCancellation(token);if(data.size()>MaximumDataLength)throw MediaError(DiagnosticCode::CpcTapeEncode);
    const auto count=std::max<std::size_t>(1,(data.size()+SegmentDataLength-1)/SegmentDataLength);std::vector<std::uint8_t> encoded(1+count*(SegmentDataLength+SegmentCrcLength)+TrailerByteCount);encoded[0]=sync;
    for(std::size_t index=0;index<count;++index){
        checkCancellation(token);const auto offset=index*SegmentDataLength;const auto size=std::min<std::size_t>(SegmentDataLength,data.size()-offset);const auto destination=1+index*(SegmentDataLength+SegmentCrcLength);
        if(size)std::copy_n(data.begin()+offset,size,encoded.begin()+destination);const auto segment=std::span(encoded).subspan(destination,SegmentDataLength);qToBigEndian(static_cast<std::uint16_t>(~Crc16::compute(segment,CrcPolynomial,CrcSeed,token)),encoded.data()+destination+SegmentDataLength);
    }
    std::fill(encoded.end()-TrailerByteCount,encoded.end(),TrailerByte);return encoded;
}
}
