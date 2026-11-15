#include "FmRead.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine::IsoFm {
namespace {
using namespace Constants;
Decoding::LocalizedText state(std::optional<bool> value){return Decoding::message(!value?UnavailableKey:*value?ValidKey:InvalidKey);}
std::vector<Decoding::LocalizedText> identity(const Reading::Header& h){using Decoding::literal;return {literal(std::to_string(h.cylinder)),literal(std::to_string(h.head)),literal(std::to_string(h.sector)),literal(std::to_string(h.size))};}
Decoding::LocalizedText headerText(const Reading::Header& h,std::optional<bool> valid){auto args=identity(h);args.push_back(Decoding::literal(std::to_string(h.code)));args.push_back(state(h.valid));args.push_back(state(valid));return Decoding::message(HeaderKey,std::move(args));}
Decoding::LocalizedText dataText(const Reading::Header& h,const Reading::Mark& mark,std::optional<bool> valid){auto args=identity(h);args.push_back(state(valid));return Decoding::message(mark.deleted()?DeletedKey:DataKey,std::move(args));}
Decoding::StructureKind kind(const Reading::Mark& mark){return mark.deleted()?Decoding::StructureKind::DeletedDataAddressMark:Decoding::StructureKind::DataAddressMark;}
struct Segment {std::uint64_t offset;std::vector<std::uint8_t> bytes;};
}
Decoding::Result decodeCore(const Flux::Bitstream& stream,std::stop_token token,std::int32_t period){
    using namespace Constants;checkCancellation(token);auto collected=Reading::collect(stream.bits(),period,token);
    Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::vector<Segment> segments;std::set<std::uint64_t> paired;
    for(std::size_t index=0;index<collected.headers.size()&&std::int64_t(collected.headers[index].offset)<period;++index){
        checkCancellation(token);const auto& h=collected.headers[index];std::vector<std::uint8_t> headerBytes{IdMark};if(h.bytes)headerBytes.insert(headerBytes.end(),h.bytes->begin(),h.bytes->end());segments.push_back({h.offset,std::move(headerBytes)});
        const auto next=index+1<collected.headers.size()?collected.headers[index+1].offset:std::uint64_t(MaximumPeriod);
        const auto found=std::lower_bound(collected.data.begin(),collected.data.end(),h.offset+HeaderBits,[](const auto& mark,auto offset){return mark.offset<offset;});
        const Reading::Mark* mark=found!=collected.data.end()&&found->offset<next?&*found:nullptr;
        const auto data=mark&&h.size!=0?Reading::data(stream.bits(),*mark,h.size,token):std::nullopt;const std::optional<bool> dataValid=data?std::optional<bool>(data->valid):std::nullopt;
        if(mark){paired.insert(mark->offset);std::vector<std::uint8_t> bytes{mark->mark};if(data)bytes.insert(bytes.end(),data->bytes.begin(),data->bytes.end());segments.push_back({mark->offset,std::move(bytes)});result.structures.push_back({kind(*mark),mark->offset,data?EncodedByteBits+data->bytes.size()*EncodedByteBits:EncodedByteBits,dataText(h,*mark,dataValid)});}
        Decoding::Sector sector;sector.cylinder=h.cylinder;sector.head=h.head;sector.number=h.sector;sector.sizeCode=h.code;sector.sizeBytes=h.size;sector.bitOffset=h.offset;
        if(h.valid==false||dataValid==false)sector.integrityValid=false;else if(data)sector.integrityValid=true;
        if(data){sector.data=data->payload;sector.recordedErrorCheckCode.assign(data->bytes.begin()+data->payload.size(),data->bytes.end());}
        if(h.bytes)sector.recordedHeaderErrorCheckCode.assign(h.bytes->begin()+HeaderFields,h.bytes->end());
        sector.headerIntegrityValid=h.valid;sector.dataIntegrityValid=dataValid;if(mark)sector.deletedDataMark=mark->deleted();result.sectors.push_back(std::move(sector));
        result.structures.push_back({Decoding::StructureKind::IdAddressMark,h.offset,std::uint64_t(h.valid?HeaderBits:EncodedByteBits),headerText(h,dataValid)});
    }
    for(const auto& mark:collected.data)if(std::int64_t(mark.offset)<period&&!paired.contains(mark.offset)){
        checkCancellation(token);segments.push_back({mark.offset,{mark.mark}});result.structures.push_back({kind(mark),mark.offset,EncodedByteBits,Decoding::message(mark.deleted()?UnpairedDeletedKey:UnpairedKey)});
    }
    std::stable_sort(segments.begin(),segments.end(),[](const auto& a,const auto& b){return a.offset<b.offset;});for(const auto& segment:segments)result.decodedBytes.insert(result.decodedBytes.end(),segment.bytes.begin(),segment.bytes.end());
    std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});
    result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceWeight,ConfidenceDivisor);return result;
}
}
