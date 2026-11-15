#include "Rx02.h"
#include "Descriptions.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::DecRx02 {
namespace {
using namespace Constants;
struct Match {std::int64_t offset;const DataMark* definition;};
struct Data {std::vector<std::uint8_t> payload;bool crcValid;std::uint64_t bitLength;};
std::optional<std::vector<std::uint8_t>> readFm(Flux::BitView bits,std::int64_t offset,std::int32_t count,std::stop_token token){
 std::vector<std::uint8_t> result;result.reserve(count);for(int i=0;i<count;++i){checkCancellation(token);const auto value=Flux::fmByte32(bits,offset+i*EncodedFmByteBitCount);if(!value)return {};result.push_back(*value);}return result;
}
std::optional<Header> readHeader(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 auto bytes=readFm(bits,offset+MarkBitCount,HeaderDecodedByteCount,token);if(!bytes)return {};std::vector<std::uint8_t> crcBytes{HeaderAddressMark};crcBytes.insert(crcBytes.end(),bytes->begin(),bytes->end());const auto valid=Crc16::compute(crcBytes,CrcPolynomial,CrcInitialValue,token)==0;return Header{(*bytes)[HeaderCylinderOffset],(*bytes)[HeaderHeadOffset],(*bytes)[HeaderSectorOffset],(*bytes)[HeaderSizeCodeOffset],valid};
}
std::optional<Match> findDataMark(Flux::BitView bits,std::int64_t start,std::stop_token token){
 const auto end=std::min(std::int64_t(bits.size())-MarkBitCount,start+MaximumDataSearchDistanceBits);for(auto offset=start;offset<=end;++offset){checkCancellation(token);for(const auto& mark:dataMarks())if(Flux::matchBytes(bits,offset,mark.pattern))return Match{offset,&mark};}return {};
}
std::optional<Data> readData(Flux::BitView bits,const Match& match,std::stop_token token){
 const auto& mark=*match.definition;const auto count=mark.sectorSize+CrcByteCount;const auto start=match.offset+MarkBitCount;auto decoded=mark.encoding==DataEncoding::M2Fm?decodeM2Fm(bits,start+PhaseBitCount,count,token):readFm(bits,start,count,token);if(!decoded)return {};
 std::vector<std::uint8_t> crcBytes{mark.mark};crcBytes.insert(crcBytes.end(),decoded->begin(),decoded->end());const auto crc=Crc16::compute(crcBytes,CrcPolynomial,CrcInitialValue,token);const auto length=mark.encoding==DataEncoding::M2Fm?MarkBitCount+PhaseBitCount+count*EncodedMfmByteBitCount:MarkBitCount+count*EncodedFmByteBitCount;return Data{std::vector<std::uint8_t>(decoded->begin(),decoded->begin()+mark.sectorSize),crc==0,std::uint64_t(length)};
}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();const auto& headerMark=FmAddressMarks::forMark(HeaderAddressMark);Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::unordered_set<std::int64_t> classified;
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,headerMark))continue;
  if(offset+HeaderBitCount>std::int64_t(bits.size())){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),MarkBitCount,Decoding::message(TruncatedKey)});offset+=MarkBitCount-1;continue;}
  const auto header=readHeader(bits,offset,token);if(!header)continue;
  if(!header->crcValid){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,headerDescription(*header,nullptr,{})});offset+=MarkBitCount-1;continue;}
  result.decodedBytes.insert(result.decodedBytes.end(),{header->cylinder,header->head,header->sector,header->sizeCode});const auto match=findDataMark(bits,offset+HeaderBitCount,token);
  if(match&&match->definition->sizeCode!=header->sizeCode){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,headerDescription(*header,match->definition,false)});offset+=MarkBitCount-1;continue;}
  auto data=match?readData(bits,*match,token):std::nullopt;
  if(match&&data){classified.insert(match->offset);result.decodedBytes.insert(result.decodedBytes.end(),data->payload.begin(),data->payload.end());result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(match->offset),data->bitLength,dataDescription(*header,*match->definition,data->crcValid)});}
  Decoding::Sector sector;sector.cylinder=header->cylinder;sector.head=header->head;sector.number=header->sector;sector.sizeCode=header->sizeCode;sector.sizeBytes=match?match->definition->sectorSize:FmSectorSize;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Crc;if(data){sector.integrityValid=data->crcValid;sector.data=std::move(data->payload);}result.sectors.push_back(std::move(sector));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,headerDescription(*header,match?match->definition:nullptr,data?std::optional(data->crcValid):std::nullopt)});offset+=MarkBitCount-1;
 }
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(classified.contains(offset))continue;const DataMark* found=nullptr;for(const auto& mark:dataMarks())if(Flux::matchBytes(bits,offset,mark.pattern)){found=&mark;break;}if(!found)continue;result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),MarkBitCount,unpairedDescription(*found)});offset+=MarkBitCount-1;}
 std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
