#include "Fm.h"
#include "../../Encoding/Bits.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Heathkit {
namespace {
using namespace Constants;
struct Header {std::uint8_t volume,cylinder,sector;bool valid;};
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
std::optional<Header> readHeader(Flux::BitView bits,std::int64_t offset,std::stop_token token){const auto encoded=Flux::mfmBytes(bits,offset+MarkBitCount,HeaderByteCount,token);if(!encoded)return {};auto record=decodeRecord(*encoded,token);return Header{record.payload[HeaderVolumeOffset],record.payload[HeaderCylinderOffset],record.payload[HeaderSectorOffset],record.valid};}
}
std::optional<Record> decodeData(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 checkCancellation(token);if(offset< -Constants::MarkBitCount||offset>std::int64_t(bits.size())-Constants::MarkBitCount)return {};auto encoded=Flux::mfmBytes(bits,offset+Constants::MarkBitCount,Constants::DataBlockByteCount,token);return encoded?std::optional(decodeRecord(*encoded,token)):std::nullopt;
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveFm(revolution.intervals,token);const auto bits=stream.bits();const auto mark=Encoding::compactFm(SyncBytes);Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::unordered_set<std::int64_t> paired,structured;
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(paired.contains(offset)||!Flux::matchBytes(bits,offset,mark))continue;
  if(offset+HeaderBitCount>std::int64_t(bits.size())){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),MarkBitCount,Decoding::message(TruncatedKey)});structured.insert(offset);offset+=MarkBitCount-1;continue;}
  const auto header=readHeader(bits,offset,token);if(!header)continue;result.decodedBytes.insert(result.decodedBytes.end(),{header->volume,header->cylinder,header->sector});const auto start=offset+HeaderBitCount,end=std::min(std::int64_t(bits.size())-MarkBitCount,start+MaximumDataSearchDistanceBits);std::int64_t dataOffset=-1;
  for(auto position=start;position<=end;++position){checkCancellation(token);if(Flux::matchBytes(bits,position,mark)){dataOffset=position;break;}}
  auto data=dataOffset<0?std::optional<Record>{}:decodeData(bits,dataOffset,token);if(dataOffset>=0)paired.insert(dataOffset);using Decoding::literal;const auto args=std::vector<Decoding::LocalizedText>{literal(std::to_string(header->cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(header->sector)),literal(std::to_string(SectorSize))};
  if(data){result.decodedBytes.insert(result.decodedBytes.end(),data->payload.begin(),data->payload.end());auto dataArgs=args;dataArgs.push_back(state(data->valid));result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),MarkBitCount+DataBlockByteCount*EncodedByteBitCount,Decoding::message(DataKey,std::move(dataArgs))});structured.insert(dataOffset);}
  Decoding::Sector sector;sector.cylinder=header->cylinder;sector.head=LogicalHead;sector.number=header->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;if(!header->valid||(data&&!data->valid))sector.integrityValid=false;else if(data)sector.integrityValid=true;if(data)sector.data=std::move(data->payload);result.sectors.push_back(std::move(sector));
  auto headerArgs=args;headerArgs.push_back(literal(std::to_string(header->volume)));headerArgs.push_back(state(header->valid));headerArgs.push_back(state(data?std::optional(data->valid):std::nullopt));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,Decoding::message(HeaderKey,std::move(headerArgs))});structured.insert(offset);offset+=MarkBitCount-1;
 }
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!Flux::matchBytes(bits,offset,mark)||paired.contains(offset)||structured.contains(offset))continue;result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),MarkBitCount,Decoding::message(UnpairedKey)});offset+=MarkBitCount-1;}
 std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
