#include "Commodore1541.h"
#include "Gcr.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Commodore1541Gcr {
namespace {
using namespace Constants;
struct Header {std::uint64_t offset,end;std::uint8_t track=0,sector=0;std::optional<bool> valid;};
struct Data {std::uint64_t offset,end;std::optional<std::vector<std::uint8_t>> bytes;std::optional<bool> valid;};
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(!valid?UnavailableKey:*valid?ValidKey:InvalidKey);}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    const auto stream=Flux::decodeNrzi(revolution.intervals,{},token);const auto bits=stream.bits();
    std::vector<Header> headers;std::vector<Data> data;Decoding::Result result;
    result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
    for(std::uint64_t offset=0;offset<bits.size();++offset){
        checkCancellation(token);if(!bits[std::size_t(offset)])continue;auto end=offset;
        while(end<bits.size()&&bits[std::size_t(end)]){checkCancellation(token);++end;}
        if(end-offset<MinimumSync){offset=end;continue;}
        result.structures.push_back({Decoding::StructureKind::CommodoreSync,offset,end-offset,Decoding::message(SyncKey)});
        if(const auto mark=CommodoreGcr::decodeByte(bits,std::int64_t(end))){
            if(*mark==HeaderMark){
                auto decoded=CommodoreGcr::decodeBytes(bits,std::int64_t(end),HeaderBytes,1,token);
                if(!decoded.bytes){result.decodedBytes.push_back(HeaderMark);headers.push_back({offset,end+EncodedByteBits,0,0,{}});}
                else{const auto& bytes=*decoded.bytes;const auto valid=bytes[0]==HeaderMark&&CommodoreGcr::validChecksum(MediaFileSystems::Bytes(bytes).subspan(HeaderChecksumOffset));headers.push_back({offset,decoded.endOffset,bytes[HeaderTrack],bytes[HeaderSector],valid});result.decodedBytes.insert(result.decodedBytes.end(),bytes.begin(),bytes.end());}
            }else if(*mark==DataMark){
                auto decoded=CommodoreGcr::decodeBytes(bits,std::int64_t(end),DataBytes,1,token);
                if(!decoded.bytes){result.decodedBytes.push_back(DataMark);data.push_back({offset,end+EncodedByteBits,{},{}});}
                else{const auto valid=(*decoded.bytes)[0]==DataMark&&CommodoreGcr::validChecksum(MediaFileSystems::Bytes(*decoded.bytes).subspan(PayloadOffset));result.decodedBytes.insert(result.decodedBytes.end(),decoded.bytes->begin(),decoded.bytes->end());data.push_back({offset,decoded.endOffset,std::move(decoded.bytes),valid});}
            }else result.decodedBytes.push_back(*mark);
        }
        offset=end;
    }
    for(const auto& block:data){checkCancellation(token);result.structures.push_back({Decoding::StructureKind::FormatData,block.offset,std::max(std::uint64_t(MinimumSync),block.end-block.offset),Decoding::message(DataKey,{state(block.valid)})});}
    for(std::size_t index=0;index<headers.size();++index){
        checkCancellation(token);const auto& header=headers[index];const auto next=index+1<headers.size()?headers[index+1].offset:MaximumBits;
        const auto found=std::upper_bound(data.begin(),data.end(),header.end,[](auto offset,const Data& value){return offset<value.offset;});const Data* block=found!=data.end()&&found->offset<next?&*found:nullptr;
        const auto dataValid=block?block->valid:std::nullopt;Decoding::Sector sector;
        sector.cylinder=header.track;sector.head=LogicalHead;sector.number=header.sector;sector.sizeCode=SizeCode;sector.sizeBytes=SectorBytes;sector.bitOffset=header.offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
        if(header.valid==false||dataValid==false)sector.integrityValid=false;else if(dataValid)sector.integrityValid=true;
        if(block&&block->bytes)sector.data=std::vector<std::uint8_t>(block->bytes->begin()+PayloadOffset,block->bytes->begin()+PayloadOffset+SectorBytes);
        result.sectors.push_back(std::move(sector));using Decoding::literal;
        result.structures.push_back({Decoding::StructureKind::CommodoreHeader,header.offset,std::max(std::uint64_t(MinimumSync),header.end-header.offset),Decoding::message(HeaderKey,{literal(std::to_string(header.track)),literal(std::to_string(LogicalHead)),literal(std::to_string(header.sector)),literal(std::to_string(SectorBytes)),state(header.valid),state(dataValid)})});
    }
    std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& left,const auto& right){return left.bitOffset<right.bitOffset;});
    result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceWeight,ConfidenceDivisor);return result;
}
}
