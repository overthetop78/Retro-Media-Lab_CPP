#include "Mfm.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Models/Sectors/SizeCodes.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Centurion {
namespace {
using namespace Constants;
struct DataResult {std::int32_t size;std::optional<bool> valid;std::optional<std::vector<std::uint8_t>> data;std::int64_t end;bool fatal;};
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
Decoding::LocalizedText truncated(std::optional<std::uint8_t> key={}){return key&&*key!=SupportedDataKey?Decoding::message(UnsupportedKey,{Decoding::literal(std::to_string(*key))}):Decoding::message(TruncatedDataKey);}
std::vector<Decoding::LocalizedText> identity(std::uint8_t cylinder,std::uint8_t sector,std::int32_t size){using Decoding::literal;return {literal(std::to_string(cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(sector)),literal(std::to_string(size))};}
std::int64_t findDataMark(Flux::BitView bits,std::int64_t start,std::stop_token token){for(auto offset=std::max(std::int64_t(0),start);offset+DataMarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(Flux::matchBytes(bits,offset,SectorMark))return -1;if(Flux::matchBytes(bits,offset,DataMark))return offset;}return -1;}
DataResult readData(Flux::BitView bits,std::int64_t offset,std::uint8_t cylinder,std::uint8_t sector,std::vector<Decoding::Structure>& structures,std::stop_token token){
 const auto prefixEnd=offset+DataPrefixBitCount;
 if(prefixEnd>std::int64_t(bits.size())){structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),DataMarkBitCount,truncated()});return {0,{},{},offset+DataMarkBitCount,false};}
 const auto prefix=Flux::mfmBytes(bits,offset+DataMarkBitCount,DataPrefixByteCount,token);if(!prefix)return {0,{},{},prefixEnd,true};const auto key=(*prefix)[DataKeyOffset];const auto size=std::int32_t(((*prefix)[DataSizeOffset]<<BitsPerByte)|(*prefix)[DataSizeOffset+1]);const auto dataEnd=prefixEnd+std::int64_t(size+CrcByteCount)*EncodedByteBitCount;
 if(key!=SupportedDataKey||size<=0||dataEnd>std::int64_t(bits.size())){structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),DataPrefixBitCount,truncated(key)});return {size,{},{},prefixEnd,false};}
 auto block=Flux::mfmBytes(bits,offset+DataMarkBitCount+EncodedByteBitCount,size+DataCrcPrefixByteCount+CrcByteCount,token);if(!block)return {size,{},{},dataEnd,true};const auto valid=Crc16::compute(*block,Crc16::Constants::Ccitt,Crc16::Constants::Zero,token)==0;std::vector<std::uint8_t> data(block->begin()+DataCrcPrefixByteCount,block->begin()+DataCrcPrefixByteCount+size);auto args=identity(cylinder,sector,size);args.push_back(Decoding::literal(std::to_string(key)));args.push_back(state(valid));structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),std::uint64_t(dataEnd-offset),Decoding::message(DataKey,std::move(args))});return {size,valid,std::move(data),dataEnd,false};
}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::unordered_set<std::int64_t> paired;
 for(std::int64_t offset=0;offset+SectorMarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,SectorMark))continue;
  if(offset+HeaderBitCount>std::int64_t(bits.size())){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),SectorMarkBitCount,Decoding::message(TruncatedHeaderKey)});offset+=SectorAdvance;continue;}
  const auto header=Flux::mfmBytes(bits,offset+SectorMarkBitCount,HeaderByteCount,token);if(!header)continue;const auto headerValid=Crc16::compute(*header,Crc16::Constants::Ccitt,Crc16::Constants::Zero,token)==0;result.decodedBytes.insert(result.decodedBytes.end(),header->begin(),header->end());const auto cylinder=(*header)[HeaderCylinderOffset],sectorNumber=(*header)[HeaderSectorOffset];const auto dataOffset=findDataMark(bits,offset+HeaderBitCount+SearchDistance,token);std::optional<DataResult> data;auto end=offset+HeaderBitCount;
  if(dataOffset>=0){paired.insert(dataOffset);data=readData(bits,dataOffset,cylinder,sectorNumber,result.structures,token);if(data->fatal)continue;if(data->data)result.decodedBytes.insert(result.decodedBytes.end(),data->data->begin(),data->data->end());end=data->end;}
  const auto size=data?data->size:0;Decoding::Sector sector;sector.cylinder=cylinder;sector.head=LogicalHead;sector.number=sectorNumber;sector.sizeCode=trySectorSizeCode(size).value_or(0);sector.sizeBytes=size;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Crc;if(!headerValid||(data&&data->valid&&!*data->valid))sector.integrityValid=false;else if(data&&data->valid)sector.integrityValid=true;if(data&&data->data)sector.data=std::move(data->data);result.sectors.push_back(std::move(sector));auto args=identity(cylinder,sectorNumber,size);args.push_back(state(headerValid));args.push_back(state(data?data->valid:std::nullopt));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,Decoding::message(HeaderKey,std::move(args))});offset=std::max(offset+SectorAdvance,end-1);
 }
 for(std::int64_t offset=0;offset+DataMarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!Flux::matchBytes(bits,offset,DataMark)||paired.contains(offset))continue;result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),DataMarkBitCount,Decoding::message(UnpairedKey)});offset+=DataAdvance;}
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
