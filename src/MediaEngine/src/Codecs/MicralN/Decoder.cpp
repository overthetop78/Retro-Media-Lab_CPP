#include "Fm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::MicralN {
std::optional<Block> decodeBlock(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(offset>std::int64_t(bits.size())-BlockBitCount)return {};const auto bytes=Flux::mfmBytes(bits,offset+MarkBitCount,BytesAfterMark,token);if(!bytes)return {};std::vector<std::uint8_t> data(bytes->begin()+DataOffset,bytes->begin()+ChecksumOffset);const auto stored=(*bytes)[ChecksumOffset];const bool valid=checksum(data,token)==stored;return Block{(*bytes)[SectorOffset],(*bytes)[CylinderOffset],std::move(data),stored,valid};
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveFm(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,SectorMark))continue;auto block=decodeBlock(bits,offset,token);
  if(!block){result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),MarkBitCount,Decoding::message(TruncatedKey)});offset+=ScanAdvance;continue;}
  result.decodedBytes.insert(result.decodedBytes.end(),block->data.begin(),block->data.end());Decoding::Sector sector;sector.cylinder=block->cylinder;sector.head=LogicalHead;sector.number=block->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.integrityValid=block->valid;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;sector.data=std::move(block->data);result.sectors.push_back(std::move(sector));using Decoding::literal;
  result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),BlockBitCount,Decoding::message(CompleteKey,{literal(std::to_string(block->cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(block->sector)),literal(std::to_string(SectorSize)),Decoding::message(block->valid?ValidKey:InvalidKey)})});offset+=ScanAdvance;
 }
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
