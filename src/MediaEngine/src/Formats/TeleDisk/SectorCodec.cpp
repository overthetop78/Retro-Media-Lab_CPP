#include "SectorCodec.h"
#include "../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::TeleDisk {
namespace {
std::string number(std::size_t n){return std::to_string(n);}
std::vector<std::string> address(std::uint8_t c,std::uint8_t h,std::uint8_t s){return {number(c),number(h),number(s)};}
}
std::uint16_t crc(MediaFileSystems::Bytes bytes,std::uint16_t initial,std::stop_token token){return Crc16::compute(bytes,Constants::CrcPolynomial,initial,token);}
std::vector<std::uint8_t> decodeSector(MediaFileSystems::Bytes encoded,SectorEncoding encoding,std::size_t expected,std::uint8_t cylinder,std::uint8_t head,std::uint8_t sector){
    using namespace Constants;std::vector<std::uint8_t> output;output.reserve(std::min(expected,BaseSectorSize<<MaximumSectorSizeCode));std::size_t total{};
    const auto args=address(cylinder,head,sector);
    auto fail=[&](DiagnosticCode code,std::initializer_list<std::string> detail){auto values=args;values.insert(values.end(),detail);throw MediaError(code,std::move(values));};
    auto append=[&](MediaFileSystems::Bytes bytes,std::size_t count){
        total+=bytes.size()*count;
        if(total<=expected)for(std::size_t i=0;i<count;++i)output.insert(output.end(),bytes.begin(),bytes.end());
    };
    auto require=[&](std::size_t offset,std::size_t count){if(count>encoded.size()-offset)fail(DiagnosticCode::Td0EncodingTruncated,{number(std::uint8_t(encoding)),number(offset),number(count),number(encoded.size()-offset)});};
    switch(encoding){
    case SectorEncoding::Raw:append(encoded,1);break;
    case SectorEncoding::RepeatedWord:{
        if(encoded.size()!=RepeatedSectorPayloadSize)fail(DiagnosticCode::Td0RepeatedPayload,{number(encoded.size()),number(RepeatedSectorPayloadSize)});
        const auto count=std::uint16_t(encoded[RepeatedSectorCountOffset]|std::uint16_t(encoded[RepeatedSectorCountOffset+1])<<BitsPerByte);
        append(encoded.subspan(RepeatedSectorPatternOffset,WordSize),count);break;
    }
    case SectorEncoding::Rle:
        for(std::size_t offset=0;offset<encoded.size();){
            require(offset,RleControlSize);const auto words=encoded[offset++],count=encoded[offset++];
            const auto length=words?std::size_t(words)*PatternWordSize:std::size_t(count);
            require(offset,length);append(encoded.subspan(offset,length),words?count:1);offset+=length;
        }break;
    default:fail(DiagnosticCode::Td0EncodingUnsupported,{number(std::uint8_t(encoding))});
    }
    if(total!=expected)fail(DiagnosticCode::Td0DecodedLength,{number(std::uint8_t(encoding)),number(total),number(expected)});
    return output;
}
EncodedSector encodeSector(MediaFileSystems::Bytes data){
    using namespace Constants;
    auto raw=[&]{return EncodedSector{SectorEncoding::Raw,{data.begin(),data.end()}};};
    if(data.size()<WordSize||data.size()%WordSize)return raw();
    for(std::size_t offset=WordSize;offset<data.size();offset+=WordSize)if(data[offset]!=data[FirstPatternByte]||data[offset+1]!=data[SecondPatternByte])return raw();
    const auto count=data.size()/WordSize;if(count>std::numeric_limits<std::uint16_t>::max())throw MediaError(DiagnosticCode::Td0RepetitionOverflow);
    std::vector<std::uint8_t> payload(RepeatedSectorPayloadSize);payload[RepeatedSectorCountOffset]=std::uint8_t(count);payload[RepeatedSectorCountOffset+1]=std::uint8_t(count>>BitsPerByte);payload[RepeatedSectorPatternOffset]=data[FirstPatternByte];payload[RepeatedSectorSecondPatternByteOffset]=data[SecondPatternByte];return {SectorEncoding::RepeatedWord,std::move(payload)};
}
}
