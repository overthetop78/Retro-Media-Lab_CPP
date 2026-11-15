#include "Mmfm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include "../../Primitives/Bits.h"
#include "../Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::HpMmfm {
std::optional<Header> decodeHeader(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(offset< -SyncBitCount||offset>std::int64_t(bits.size())-SyncBitCount)return {};const auto id=decodeBytes(bits,offset+SyncBitCount,HeaderByteCount,token);if(!id)return {};const auto cylinder=Primitives::reverseBits((*id)[HeaderCylinderOffset]),encodedSector=Primitives::reverseBits((*id)[HeaderSectorOffset]);const auto valid=Crc16::compute(*id,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)==0;return Header{cylinder,std::uint8_t(encodedSector>>HeadShift),std::uint8_t(encodedSector&SectorMask),valid};
}
std::int64_t findDataSync(Flux::BitView bits,std::int64_t headerOffset,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(headerOffset>std::int64_t(bits.size())||headerOffset< -SyncBitCount-MaximumDataSearchOffsetBits)return -1;
 const auto start=headerOffset+SyncBitCount+MinimumDataSearchOffsetBits,end=std::min(std::int64_t(bits.size()),headerOffset+SyncBitCount+MaximumDataSearchOffsetBits);for(auto offset=start;offset+SyncBitCount<=end;++offset){checkCancellation(token);if(Flux::matchBytes(bits,offset,DataSync))return offset;}return -1;
}
std::optional<Data> decodeData(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(offset< -SyncBitCount||offset>std::int64_t(bits.size())-SyncBitCount)return {};const auto encoded=decodeBytes(bits,offset+SyncBitCount,EncodedDataByteCount,token);if(!encoded)return {};const auto valid=Crc16::compute(*encoded,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)==0;return Data{decodePayload(MediaFileSystems::Bytes(*encoded).first(SectorSize),token),valid};
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::unordered_set<std::int64_t> used;
 for(std::int64_t offset=0;offset+HeaderBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,SectorSync))continue;const auto header=decodeHeader(bits,offset,token);if(!header)continue;const auto dataOffset=findDataSync(bits,offset,token);auto data=dataOffset>=0&&used.insert(dataOffset).second?decodeData(bits,dataOffset,token):std::nullopt;using Decoding::literal;
  auto args=std::vector<Decoding::LocalizedText>{literal(std::to_string(header->cylinder)),literal(std::to_string(header->head)),literal(std::to_string(header->sector)),literal(std::to_string(SectorSize))};
  if(data){result.decodedBytes.insert(result.decodedBytes.end(),data->payload.begin(),data->payload.end());auto dataArgs=args;dataArgs.push_back(Decoding::message(data->crcValid?ValidKey:InvalidKey));result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),SyncBitCount+EncodedDataByteCount*EncodedByteBitCount,Decoding::message(DataKey,std::move(dataArgs))});}
  Decoding::Sector sector;sector.cylinder=header->cylinder;sector.head=header->head;sector.number=header->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Crc;if(!header->crcValid||(data&&!data->crcValid))sector.integrityValid=false;else if(data)sector.integrityValid=true;if(data)sector.data=std::move(data->payload);result.sectors.push_back(std::move(sector));args.push_back(Decoding::message(header->crcValid?ValidKey:InvalidKey));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,Decoding::message(HeaderKey,std::move(args))});offset+=SyncBitCount-1;
 }
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
