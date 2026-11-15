#include "Fm.h"
#include "../../Encoding/Bits.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::DataGeneral {
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveFm(revolution.intervals,token);const auto bits=stream.bits();const auto sync=Encoding::compactFm(SyncBytes);Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::vector<std::int64_t> offsets;
 for(std::int64_t offset=0;offset+EncodedSyncBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(Flux::matchBytes(bits,offset,sync))offsets.push_back(offset);}
 for(std::size_t index=0;index+1<offsets.size();++index){
  checkCancellation(token);const auto headerOffset=offsets[index],dataOffset=offsets[index+1],headerStart=headerOffset+EncodedSyncBitCount,distance=dataOffset-headerStart;if(distance<MinimumSyncDistance||distance>MaximumSyncDistance)continue;
  const auto identity=Flux::mfmBytes(bits,headerStart,IdentityByteCount,token);if(!identity)continue;const auto sectorNumber=std::uint8_t((*identity)[SectorOffset]>>SectorShift);if(sectorNumber>MaximumSectorNumber)continue;auto block=Flux::mfmBytes(bits,dataOffset+EncodedSyncBitCount,DataBlockByteCount,token);if(!block)continue;
  const auto stored=std::uint16_t(((*block)[ChecksumHighOffset]<<BitsPerByte)|(*block)[ChecksumLowOffset]);const auto payload=MediaFileSystems::Bytes(*block).first(SectorSize);const auto valid=checksum(payload,token)==stored;Decoding::Sector sector;sector.cylinder=std::uint8_t((*identity)[CylinderAndHeadOffset]&CylinderMask);sector.head=std::uint8_t(((*identity)[CylinderAndHeadOffset]&HeadMask)>>HeadShift);sector.number=sectorNumber;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.integrityValid=valid;sector.bitOffset=headerOffset;sector.integrityKind=Decoding::IntegrityKind::Checksum;sector.data=std::vector<std::uint8_t>(payload.begin(),payload.end());result.decodedBytes.insert(result.decodedBytes.end(),payload.begin(),payload.end());using Decoding::literal;
  auto args=std::vector<Decoding::LocalizedText>{literal(std::to_string(sector.cylinder)),literal(std::to_string(sector.head)),literal(std::to_string(sector.number)),literal(std::to_string(SectorSize))};result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(headerOffset),EncodedSyncBitCount+IdentityByteCount*EncodedByteBitCount,Decoding::message(HeaderKey,args)});args.push_back(Decoding::message(valid?ValidKey:InvalidKey));result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),EncodedSyncBitCount+DataBlockByteCount*EncodedByteBitCount,Decoding::message(DataKey,std::move(args))});result.sectors.push_back(std::move(sector));
  const auto dataEnd=dataOffset+EncodedSyncBitCount+DataBlockByteCount*EncodedByteBitCount;while(index+1<offsets.size()&&offsets[index+1]<dataEnd){checkCancellation(token);++index;}
 }
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
