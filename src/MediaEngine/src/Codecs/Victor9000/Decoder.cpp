#include "Gcr.h"
#include "../Commodore/Constants.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Victor9000 {
namespace {Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?Constants::ValidKey:Constants::InvalidKey):Constants::UnavailableKey);}}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveDoubledNrzi(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::vector<std::int64_t> paired;
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,HeaderMark))continue;const auto header=decodeHeader(bits,offset,token);if(header)result.decodedBytes.insert(result.decodedBytes.end(),header->bytes.begin(),header->bytes.end());const auto headerEnd=offset+EncodedDataStartBitOffset+HeaderByteCount*CommodoreGcr::Constants::ByteBits*EncodedCellStride;
  const auto dataOffset=findDataMark(bits,headerEnd,std::min(std::int64_t(bits.size()),offset+MaximumDataSearchDistanceBits),token);auto data=dataOffset<0?std::optional<Data>{}:decodeData(bits,dataOffset,token);
  using Decoding::literal;const auto coords=std::vector<Decoding::LocalizedText>{literal(std::to_string(header?header->cylinder:0)),literal(std::to_string(LogicalHead)),literal(std::to_string(header?header->sector:0)),literal(std::to_string(SectorByteCount))};
  if(dataOffset>=0){paired.push_back(dataOffset);if(!data)result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),MarkBitCount,Decoding::message(TruncatedKey)});else{result.decodedBytes.insert(result.decodedBytes.end(),data->payload.begin(),data->payload.end());auto args=coords;args.push_back(literal(std::string{HexDigits[data->prefix>>HexShift],HexDigits[data->prefix&HexMask]}));args.push_back(state(data->valid));result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),data->endOffset-std::uint64_t(dataOffset),Decoding::message(DataKey,std::move(args))});}}
  Decoding::Sector sector;sector.cylinder=header?header->cylinder:0;sector.head=LogicalHead;sector.number=header?header->sector:0;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorByteCount;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
  if((header&&!header->valid)||(data&&!data->valid))sector.integrityValid=false;else if(data)sector.integrityValid=true;if(data)sector.data=std::move(data->payload);result.sectors.push_back(std::move(sector));auto args=coords;args.push_back(state(header?std::optional(header->valid):std::nullopt));args.push_back(state(data?std::optional(data->valid):std::nullopt));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),std::uint64_t(std::max(std::int64_t(MarkBitCount),headerEnd-offset)),Decoding::message(HeaderKey,std::move(args))});offset=std::max(offset+MarkBitCount-1,std::int64_t(data?data->endOffset:std::uint64_t(headerEnd))-1);
 }
 collectUnpairedData(bits,paired,result.structures,token);result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
