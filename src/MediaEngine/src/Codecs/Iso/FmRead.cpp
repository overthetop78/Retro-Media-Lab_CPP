#include "FmRead.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Models/Sectors/SizeCodes.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoFm::Reading {
using namespace Constants;
namespace {
bool clocks(Flux::BitView bits,std::uint64_t offset,std::uint64_t count,std::stop_token token){
    if(offset>bits.size()||count>(bits.size()-offset)/EncodedByteBits)return false;
    const auto end=offset+count*EncodedByteBits;
    for(auto bit=offset;bit<end;bit+=CellsPerDataBit){checkCancellation(token);if(!bits[std::size_t(bit)])return false;}return true;
}
}
std::optional<std::uint8_t> recognize(Flux::BitView bits,std::int64_t offset){
    if(Flux::match(bits,offset,IdPattern))return IdMark;
    if(Flux::match(bits,offset,DataPattern))return DataMark;
    if(Flux::match(bits,offset,DeletedPattern))return DeletedMark;
    return {};
}
Header header(Flux::BitView bits,std::uint64_t offset,std::stop_token token){
    checkCancellation(token);Header result;result.offset=offset;
    if(offset>bits.size()||HeaderBits>bits.size()-offset)return result;
    auto bytes=Flux::mfmBytes(bits,std::int64_t(offset+EncodedByteBits),HeaderBytesAfterMark,token);if(!bytes)return result;
    result.cylinder=(*bytes)[HeaderCylinder];result.head=(*bytes)[HeaderHead];result.sector=(*bytes)[HeaderSector];result.code=(*bytes)[HeaderCode];result.size=result.code<=MaximumSizeCode?sectorSizeBytes(result.code):0;
    const auto stored=std::uint16_t(((*bytes)[HeaderFields]<<Crc16::Constants::ByteBits)|(*bytes)[HeaderFields+1]);
    const std::vector<std::uint8_t> record{IdMark,result.cylinder,result.head,result.sector,result.code};
    result.valid=stored==Crc16::compute(record,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)&&clocks(bits,offset+EncodedByteBits,HeaderBytesAfterMark,token);result.bytes=std::move(bytes);return result;
}
std::optional<Data> data(Flux::BitView bits,const Mark& mark,std::uint32_t size,std::stop_token token){
    checkCancellation(token);if(mark.offset>bits.size()||EncodedByteBits>bits.size()-mark.offset)return {};
    auto bytes=Flux::mfmBytes(bits,std::int64_t(mark.offset+EncodedByteBits),std::int64_t(size)+CrcBytes,token);if(!bytes)return {};
    const auto stored=std::uint16_t(((*bytes)[size]<<Crc16::Constants::ByteBits)|(*bytes)[size+1]);
    std::vector<std::uint8_t> payload(bytes->begin(),bytes->begin()+size),record{mark.mark};record.insert(record.end(),payload.begin(),payload.end());
    const auto valid=stored==Crc16::compute(record,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)&&clocks(bits,mark.offset+EncodedByteBits,bytes->size(),token);return Data{std::move(payload),std::move(*bytes),valid};
}
Collection collect(Flux::BitView bits,std::int32_t period,std::stop_token token){
    Collection result;if(period<=0)return result;const auto end=std::uint64_t(period)*CellsPerDataBit;
    for(std::uint64_t offset=0;offset<=bits.size()&&EncodedByteBits<=bits.size()-offset&&offset<end;++offset){
        checkCancellation(token);const auto mark=recognize(bits,std::int64_t(offset));if(!mark)continue;
        if(*mark==IdMark)result.headers.push_back(header(bits,offset,token));else result.data.push_back({offset,*mark});
        offset+=*mark==IdMark&&HeaderBits<=bits.size()-offset?HeaderBits-1:EncodedByteBits-1;
    }
    return result;
}
}
