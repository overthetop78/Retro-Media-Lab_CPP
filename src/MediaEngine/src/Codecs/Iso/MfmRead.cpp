#include "MfmRead.h"
#include <algorithm>
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Models/Sectors/SizeCodes.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoMfm::Reading {
using namespace Constants;
std::optional<std::uint8_t> recognize(Flux::BitView bits,std::int64_t offset){
    if(offset<0||std::uint64_t(offset)>bits.size()||SyncAndMarkBits>bits.size()-std::uint64_t(offset))return {};
    for(int index=0;index<SyncByteCount;++index)if(!Flux::match(bits,offset+index*EncodedByteBits,SyncPattern))return {};
    return Flux::mfmByte(bits,offset+SyncBits);
}
Header header(Flux::BitView bits,std::uint64_t offset,std::stop_token token){
    checkCancellation(token);Header result;result.offset=offset;
    if(offset>bits.size()||HeaderBits>bits.size()-offset)return result;
    auto bytes=Flux::mfmBytes(bits,std::int64_t(offset+SyncAndMarkBits),HeaderBytesAfterMark,token);if(!bytes)return result;
    result.cylinder=(*bytes)[HeaderCylinder];result.head=(*bytes)[HeaderHead];result.sector=(*bytes)[HeaderSector];result.code=(*bytes)[HeaderCode];result.size=result.code<=MaximumSizeCode?sectorSizeBytes(result.code):0;
    const auto stored=std::uint16_t(((*bytes)[HeaderFields]<<Crc16::Constants::ByteBits)|(*bytes)[HeaderFields+1]);
    const std::vector<std::uint8_t> record{SyncByte,SyncByte,SyncByte,IdMark,result.cylinder,result.head,result.sector,result.code};
    result.valid=stored==Crc16::compute(record,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token);result.bytes=std::move(bytes);return result;
}
std::optional<Data> data(Flux::BitView bits,const Mark& mark,std::uint32_t size,std::stop_token token){
    checkCancellation(token);if(mark.offset>bits.size()||SyncAndMarkBits>bits.size()-mark.offset)return {};
    auto bytes=Flux::mfmBytes(bits,std::int64_t(mark.offset+SyncAndMarkBits),std::int64_t(size)+CrcBytes,token);if(!bytes)return {};
    const auto stored=std::uint16_t(((*bytes)[size]<<Crc16::Constants::ByteBits)|(*bytes)[size+1]);
    std::vector<std::uint8_t> payload(bytes->begin(),bytes->begin()+size),record(SyncByteCount,SyncByte);record.push_back(mark.mark);record.insert(record.end(),payload.begin(),payload.end());
    const auto valid=stored==Crc16::compute(record,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token);return Data{std::move(payload),std::move(*bytes),valid};
}
Collection collect(Flux::BitView bits,std::uint64_t length,std::stop_token token){
    Collection result;
    for(std::uint64_t offset=0;offset<=length&&SyncAndMarkBits<=length-offset;++offset){
        checkCancellation(token);const auto mark=recognize(bits,std::int64_t(offset));if(!mark)continue;
        if(*mark==IdMark)result.headers.push_back(header(bits,offset,token));else if(*mark==DataMark||*mark==DeletedMark)result.data.push_back({offset,*mark});else result.unknown.push_back({offset,*mark});
        offset+=*mark==IdMark&&HeaderBits<=bits.size()-offset?HeaderBits-1:SyncBits-1;
    }
    const auto tail=std::min(length,std::uint64_t(CircularTailBits));const auto count=result.data.size();
    for(std::size_t index=0;index<count;++index)if(result.data[index].offset<tail)result.data.push_back({result.data[index].offset+length,result.data[index].mark});return result;
}
}
