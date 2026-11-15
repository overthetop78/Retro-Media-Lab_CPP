#include "Gcr.h"
#include "../Commodore/Gcr.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Victor9000 {
std::uint16_t checksum(MediaFileSystems::Bytes data,std::stop_token token){checkCancellation(token);std::uint16_t sum=0;for(auto byte:data){checkCancellation(token);sum=std::uint16_t(sum+byte);}return sum;}
void addBlock(Flux::Bits& target,MediaFileSystems::Bytes marker,MediaFileSystems::Bytes values,std::stop_token token){
 checkCancellation(token);if(marker.size()*Constants::BitsPerByte<Constants::EncodedDataStartBitOffset)throw MediaError(DiagnosticCode::VictorMarkerLength,{std::to_string(marker.size()*Constants::BitsPerByte)});
 Flux::Bits block;Encoding::raw(block,marker);CommodoreGcr::write(block,Constants::EncodedDataStartBitOffset,values,Constants::EncodedCellStride,token);target.insert(target.end(),block.begin(),block.end());
}
std::optional<Header> decodeHeader(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 using namespace Constants;auto decoded=CommodoreGcr::decodeBytes(bits,offset+EncodedDataStartBitOffset,HeaderByteCount,EncodedCellStride,token);if(!decoded.bytes)return {};auto& bytes=*decoded.bytes;
 const bool valid=bytes[HeaderTypeOffset]==HeaderType&&bytes[HeaderId2Offset]==HeaderId2&&bytes[HeaderId1Offset]==HeaderId1&&bytes[HeaderSumOffset]==headerChecksum(bytes[HeaderCylinderOffset],bytes[HeaderSectorOffset]);return Header{bytes[HeaderCylinderOffset],bytes[HeaderSectorOffset],std::move(bytes),valid};
}
std::optional<Data> decodeData(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 using namespace Constants;auto decoded=CommodoreGcr::decodeBytes(bits,offset+EncodedDataStartBitOffset,DecodedDataByteCount,EncodedCellStride,token);if(!decoded.bytes)return {};const auto& bytes=*decoded.bytes;std::vector<std::uint8_t> payload(bytes.begin()+DataOffset,bytes.begin()+ChecksumLowOffset);const auto stored=std::uint16_t(bytes[ChecksumLowOffset]|(std::uint16_t(bytes[ChecksumHighOffset])<<BitsPerByte));const bool valid=checksum(payload,token)==stored;return Data{bytes[DataPrefixOffset],std::move(payload),stored,valid,decoded.endOffset};
}
std::int64_t findDataMark(Flux::BitView bits,std::int64_t start,std::int64_t end,std::stop_token token){
 checkCancellation(token);for(auto offset=std::max(std::int64_t(0),start);offset+Constants::MarkBitCount<=end;++offset){checkCancellation(token);if(Flux::matchBytes(bits,offset,Constants::DataMark))return offset;}return -1;
}
void collectUnpairedData(Flux::BitView bits,std::span<const std::int64_t> paired,std::vector<Decoding::Structure>& structures,std::stop_token token){
 checkCancellation(token);const std::unordered_set<std::int64_t> pairedOffsets(paired.begin(),paired.end());for(std::int64_t offset=0;offset+Constants::MarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!Flux::matchBytes(bits,offset,Constants::DataMark)||pairedOffsets.contains(offset))continue;structures.push_back({Decoding::StructureKind::FormatData,std::uint64_t(offset),Constants::MarkBitCount,Decoding::message(Constants::UnpairedKey)});offset+=Constants::MarkBitCount-1;}
}
}
