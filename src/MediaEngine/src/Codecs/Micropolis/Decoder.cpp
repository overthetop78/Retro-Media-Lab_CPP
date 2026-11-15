#include "Mfm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Micropolis {
std::optional<Record> decodeRecord(Flux::BitView bits,std::int64_t offset,std::stop_token token){checkCancellation(token);if(offset>std::int64_t(bits.size())-Constants::RecordByteCount*Constants::EncodedByteBitCount)return {};const auto bytes=Flux::mfmBytes(bits,offset,Constants::RecordByteCount,token);return bytes?parseRecord(*bytes,token):std::nullopt;}
Decoding::Result decode(const Flux::Revolution& revolution,bool includeRecord,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
 for(std::int64_t offset=0;offset+SyncBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,Sync))continue;auto record=decodeRecord(bits,offset+SyncZeroCount*EncodedByteBitCount,token);
  if(!record){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),SyncBitCount,Decoding::message(TruncatedKey)});offset+=SyncBitCount-1;continue;}
  result.decodedBytes.insert(result.decodedBytes.end(),record->data.begin(),record->data.end());const auto valid=record->checksumValid(token);Decoding::Sector sector;sector.cylinder=record->cylinder;sector.head=LogicalHead;sector.number=record->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=std::uint32_t(includeRecord?record->bytes.size():record->data.size());sector.integrityValid=valid;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;sector.data=includeRecord?std::move(record->bytes):std::move(record->data);result.sectors.push_back(std::move(sector));using Decoding::literal;
  result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),(SyncZeroCount+RecordByteCount)*EncodedByteBitCount,Decoding::message(CompleteKey,{literal(std::to_string(record->cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(record->sector)),literal(std::to_string(SectorSize)),Decoding::message(valid?ValidKey:InvalidKey)})});offset+=SyncBitCount-1;
 }
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
