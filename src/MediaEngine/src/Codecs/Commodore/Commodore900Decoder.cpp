#include "Commodore900.h"
#include "Gcr.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine::Commodore900Gcr {
namespace {
using namespace Constants;
struct Header {std::uint64_t offset,end;std::uint8_t cylinder,sector;bool valid;};
struct Data {std::uint64_t offset,end;std::vector<std::uint8_t> bytes;bool valid;};
Decoding::LocalizedText state(bool valid){return Decoding::message(valid?ValidKey:InvalidKey);}
Decoding::LocalizedText description(std::string_view key,const Header& header,bool valid){using Decoding::literal;return Decoding::message(key,{literal(std::to_string(header.cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(header.sector)),literal(std::to_string(SectorBytes)),state(valid)});}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeNrzi(revolution.intervals,{},token);const auto bits=stream.bits();std::vector<Header> headers;std::vector<Data> data;Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
 for(std::uint64_t offset=0;offset<bits.size();++offset){
  checkCancellation(token);if(!bits[std::size_t(offset)])continue;auto end=offset;while(end<bits.size()&&bits[std::size_t(end)]){checkCancellation(token);++end;}
  if(end-offset<MinimumSync){offset=end;continue;}
  result.structures.push_back({Decoding::StructureKind::CommodoreSync,offset,end-offset,Decoding::message(SyncKey)});
  const auto header=CommodoreGcr::decodeBytes(bits,std::int64_t(end),HeaderBytes,1,token);
  if(header.bytes&&(*header.bytes)[0]==HeaderMark){
   const auto& bytes=*header.bytes;headers.push_back({offset,header.endOffset,bytes[HeaderCylinder],bytes[HeaderSector],CommodoreGcr::validChecksum(bytes)});result.decodedBytes.insert(result.decodedBytes.end(),bytes.begin(),bytes.end());
  }else{
   auto record=CommodoreGcr::decodeBytes(bits,std::int64_t(end),DataBytes,1,token);
   if(record.bytes&&(*record.bytes)[0]==DataMark){const auto valid=CommodoreGcr::validChecksum(*record.bytes);result.decodedBytes.insert(result.decodedBytes.end(),record.bytes->begin(),record.bytes->end());data.push_back({offset,record.endOffset,std::move(*record.bytes),valid});}
  }
  offset=end;
 }
 std::set<std::size_t> paired;
 for(std::size_t index=0;index<headers.size();++index){
  checkCancellation(token);const auto& header=headers[index];const auto next=index+1<headers.size()?headers[index+1].offset:MaximumBits;const Data* record=nullptr;
  const auto candidate=std::upper_bound(data.begin(),data.end(),header.end,[](auto offset,const Data& value){return offset<value.offset;});if(candidate!=data.end()&&candidate->offset<next){record=&*candidate;paired.insert(std::size_t(candidate-data.begin()));}
  Decoding::Sector sector;sector.cylinder=header.cylinder;sector.head=LogicalHead;sector.number=header.sector;sector.sizeCode=SizeCode;sector.sizeBytes=SectorBytes;sector.bitOffset=header.offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
  if(!header.valid||(record&&!record->valid))sector.integrityValid=false;else if(record)sector.integrityValid=true;
  if(record)sector.data=std::vector<std::uint8_t>(record->bytes.begin()+PayloadOffset,record->bytes.begin()+PayloadOffset+SectorBytes);result.sectors.push_back(std::move(sector));
  result.structures.push_back({Decoding::StructureKind::CommodoreHeader,header.offset,std::max(std::uint64_t(MinimumSync),header.end-header.offset),description(HeaderKey,header,header.valid)});
  if(record)result.structures.push_back({Decoding::StructureKind::FormatData,record->offset,std::max(std::uint64_t(MinimumSync),record->end-record->offset),description(DataKey,header,record->valid)});
 }
 for(std::size_t index=0;index<data.size();++index){checkCancellation(token);if(!paired.contains(index)){const auto& record=data[index];result.structures.push_back({Decoding::StructureKind::FormatData,record.offset,std::max(std::uint64_t(MinimumSync),record.end-record.offset),Decoding::message(UnpairedKey,{state(record.valid)})});}}
 const auto valid=std::count_if(result.sectors.begin(),result.sectors.end(),[](const auto& sector){return sector.integrityValid==true;});result.confidence=Decoding::confidence(std::int32_t(valid),0,1,ExpectedSectors);
 std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});return result;
}
}
