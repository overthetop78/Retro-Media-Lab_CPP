#include "Mfm.h"
#include "../RotatingChecksum.h"
#include "../../Encoding/Bits.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine::Northstar {
std::optional<Identity> decodeIdentity(Flux::BitView bits,std::int64_t offset,std::stop_token token){checkCancellation(token);const auto packed=Flux::mfmByte(bits,offset);if(!packed)return {};const auto a=unpack(*packed);return Identity{a.cylinder,a.sector,*packed};}
std::optional<Block> decodeBlock(Flux::BitView bits,std::int64_t offset,Identity identity,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(offset>std::int64_t(bits.size())-IdentityBitCount-PayloadBitCount-EncodedByteBitCount)return {};
 auto data=Flux::mfmBytes(bits,offset+IdentityBitCount,SectorSize,token);if(!data)return {};const auto stored=Flux::mfmByte(bits,offset+IdentityBitCount+PayloadBitCount);if(!stored)return {};const bool valid=RotatingChecksum::compute(*data,token)==*stored;return Block{identity,std::move(*data),*stored,valid};
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();const auto mark=Encoding::compactMfm(SyncBytes);Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,mark))continue;auto identity=decodeIdentity(bits,offset+MarkBitCount,token);auto block=identity?decodeBlock(bits,offset+MarkBitCount,*identity,token):std::nullopt;
  if(block){result.decodedBytes.push_back(identity->packed);result.decodedBytes.insert(result.decodedBytes.end(),block->data.begin(),block->data.end());}
  Decoding::LocalizedText description=Decoding::message(TruncatedKey);std::uint64_t length=MarkBitCount;
  if(identity){Decoding::Sector sector;sector.cylinder=identity->cylinder;sector.head=LogicalHead;sector.number=identity->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;if(block){sector.integrityValid=block->checksumValid;sector.data=std::move(block->data);}sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;result.sectors.push_back(std::move(sector));using Decoding::literal;
   description=Decoding::message(HeaderKey,{literal(std::to_string(identity->cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(identity->sector)),literal(std::to_string(SectorSize)),Decoding::message(block?(block->checksumValid?ValidKey:InvalidKey):UnavailableKey)});length=block?FullBlockBitCount:MarkBitCount+IdentityBitCount;
  }
  result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),length,std::move(description)});offset+=ScanAdvance;
 }
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
std::vector<Decoding::Sector> decodeHardSectors(const Flux::Revolution& revolution,bool fm,std::int32_t cylinder,std::int32_t head,std::int32_t sectorCount,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=fm?Flux::decodeAdaptiveFm(revolution.intervals,token):Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();const auto sync=fm?Encoding::compactFm(HardSyncBytes):Encoding::compactMfm(HardSyncBytes);const auto size=fm?FmSectorSize:SectorSize;
 const auto rawCells=revolution.indexTimeTicks/stream.cellTicks(),lower=std::floor(rawCells),fraction=rawCells-lower;const auto rounded=fraction>Half?lower+1:fraction<Half?lower:std::fmod(lower,Two)==0?lower:lower+1;
 if(!std::isfinite(rounded)||rounded<0||rounded>MaximumCellCount)throw MediaError(DiagnosticCode::NorthstarWindowCapacity);const auto cells=std::int64_t(rounded);std::vector<Decoding::Sector> sectors;
 for(std::int64_t number=0;number<sectorCount;++number){
  checkCancellation(token);const auto start=cells*number/sectorCount,end=std::min(std::int64_t(bits.size()),cells*(number+1)/sectorCount);if(start>=std::int64_t(bits.size()))break;std::optional<Decoding::Sector> candidate;
  for(auto offset=start;offset+(size+(fm?FmWindowExtraBytes:MfmWindowExtraBytes))*EncodedByteBitCount<=end;++offset){
   checkCancellation(token);if(!Flux::matchBytes(bits,offset,sync))continue;const auto dataOffset=offset+(fm?FmDataByteOffset:MfmDataByteOffset)*EncodedByteBitCount;auto data=Flux::mfmBytes(bits,dataOffset,size,token);if(!data)continue;const auto stored=Flux::mfmByte(bits,dataOffset+size*EncodedByteBitCount);if(!stored)continue;const auto valid=RotatingChecksum::compute(*data,token)==*stored;
   if(cylinder<0||cylinder>MaximumAddress||head<0||head>MaximumAddress)throw MediaError(DiagnosticCode::NorthstarHardAddressRange,{std::to_string(cylinder),std::to_string(head)});
   Decoding::Sector sector;sector.cylinder=std::uint8_t(cylinder);sector.head=std::uint8_t(head);sector.number=std::int32_t(number);sector.sizeCode=fm?FmSectorSizeCode:SectorSizeCode;sector.sizeBytes=size;sector.integrityValid=valid;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;sector.data=std::move(*data);candidate=std::move(sector);if(valid)break;
  }
  if(candidate)sectors.push_back(std::move(*candidate));
 }
 return sectors;
}
}
