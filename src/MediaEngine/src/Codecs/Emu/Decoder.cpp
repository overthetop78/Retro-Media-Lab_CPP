#include "Fm.h"
#include "../Crc16.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::EmuFm {
namespace {
using namespace Constants;
struct Identity {std::uint8_t cylinder,head;};
std::optional<std::vector<std::uint8_t>> readBytes(Flux::BitView bits,std::int64_t offset,std::int32_t count,std::stop_token token){std::vector<std::uint8_t> result;result.reserve(count);for(std::int32_t i=0;i<count;++i){checkCancellation(token);const auto byte=Flux::fmByte32(bits,offset+i*EncodedByteBitCount);if(!byte)return {};result.push_back(*byte);}return result;}
std::optional<Identity> readHeader(Flux::BitView bits,std::int64_t offset,std::stop_token token){const auto header=readBytes(bits,offset+MarkBitCount,HeaderDecodedByteCount,token);if(!header||Crc16::compute(*header,Crc16::Constants::Ibm,Crc16::Constants::Zero,token)!=0)return {};const auto track=reverseBits(header->front());return Identity{std::uint8_t(track>>TrackShift),std::uint8_t(track&HeadMask)};}
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
}
std::optional<Data> decodeData(Flux::BitView bits,std::int64_t offset,std::stop_token token){checkCancellation(token);const auto block=readBytes(bits,offset+Constants::MarkBitCount,Constants::DataBlockByteCount,token);if(!block)return {};const bool valid=Crc16::compute(*block,Crc16::Constants::Ibm,Crc16::Constants::Zero,token)==0;return Data{std::vector<std::uint8_t>(block->begin(),block->begin()+Constants::SectorSize),valid};}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::unordered_set<std::int64_t> classified;
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,SectorMark)||offset+HeaderBitCount>std::int64_t(bits.size()))continue;const auto identity=readHeader(bits,offset,token);if(!identity)continue;classified.insert(offset);result.decodedBytes.push_back(std::uint8_t((identity->cylinder<<TrackShift)|identity->head));
  const auto start=offset+HeaderBitCount,end=std::min(std::int64_t(bits.size())-MarkBitCount,start+MaximumDataSearchDistanceBits);std::int64_t dataOffset=-1;for(auto position=start;position<=end;++position){checkCancellation(token);if(Flux::matchBytes(bits,position,SectorMark)){dataOffset=position;break;}}auto data=dataOffset<0?std::optional<Data>{}:decodeData(bits,dataOffset,token);
  using Decoding::literal;const auto args=std::vector<Decoding::LocalizedText>{literal(std::to_string(identity->cylinder)),literal(std::to_string(identity->head)),literal(std::to_string(SectorNumber)),literal(std::to_string(SectorSize))};
  if(data){classified.insert(dataOffset);result.decodedBytes.insert(result.decodedBytes.end(),data->payload.begin(),data->payload.end());auto dataArgs=args;dataArgs.push_back(state(data->valid));result.structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(dataOffset),MarkBitCount+DataBlockByteCount*EncodedByteBitCount,Decoding::message(DataKey,std::move(dataArgs))});}
  Decoding::Sector sector;sector.cylinder=identity->cylinder;sector.head=identity->head;sector.number=SectorNumber;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Crc;if(data){sector.integrityValid=data->valid;sector.data=std::move(data->payload);}result.sectors.push_back(std::move(sector));auto headerArgs=args;headerArgs.push_back(state(true));headerArgs.push_back(state(data?std::optional(data->valid):std::nullopt));result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),HeaderBitCount,Decoding::message(HeaderKey,std::move(headerArgs))});offset+=MarkBitCount-1;
 }
 for(std::int64_t offset=0;offset+MarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!classified.contains(offset)&&Flux::matchBytes(bits,offset,SectorMark))result.structures.push_back({Decoding::StructureKind::FormatHeader,std::uint64_t(offset),MarkBitCount,Decoding::message(UnknownKey)});}
 std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
