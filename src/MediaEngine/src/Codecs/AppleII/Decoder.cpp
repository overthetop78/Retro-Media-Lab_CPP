#include "Gcr.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine::AppleII {
namespace {
using namespace Constants;
using Decoding::literal;
struct Address {std::uint8_t volume,cylinder,sector;bool valid;};
struct Data {std::optional<std::vector<std::uint8_t>> payload;std::optional<bool> valid;std::int64_t end;};
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
std::optional<Address> address(Flux::BitView bits,std::int64_t offset,Decoding::Result& result,bool append){
 offset+=PrologueBitCount;if(offset+EncodedAddressBitCount>std::int64_t(bits.size()))return {};
 std::array<std::uint8_t,AddressValueCount> values{};
 for(std::int32_t i=0;i<AddressValueCount;++i)values[i]=decodeFourAndFour(*Flux::rawByte(bits,offset+i*EncodedBytesPerAddressValue*BitsPerByte),*Flux::rawByte(bits,offset+(i*EncodedBytesPerAddressValue+1)*BitsPerByte));
 if(append)result.decodedBytes.insert(result.decodedBytes.end(),values.begin(),values.end());
 return Address{values[VolumeOffset],values[CylinderOffset],values[SectorOffset],addressChecksum(values[VolumeOffset],values[CylinderOffset],values[SectorOffset])==values[ChecksumOffset]};
}
Data findData(Flux::BitView bits,std::int64_t headerEnd,const std::optional<Address>& addr,bool five,Decoding::Result& result,std::set<std::int64_t>& paired,std::stop_token token){
 const auto end=std::min(std::int64_t(bits.size()),headerEnd+DataSearchBitCount);std::int64_t position=-1;
 for(auto offset=std::max(std::int64_t(0),headerEnd);offset+PrologueBitCount<=end;++offset){checkCancellation(token);if(Flux::match(bits,offset,DataPrologue,PrologueBitCount)){position=offset;break;}}
 if(position<0)return {{},{},headerEnd};paired.insert(position);
 auto decoded=five?decodeFiveAndThree(bits,position+PrologueBitCount,token):decodeSixAndTwo(bits,position+PrologueBitCount,token);
 if(!decoded){if(!five)result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(position),PrologueBitCount,Decoding::message(TruncatedKey)});return {{},{},headerEnd};}
 result.decodedBytes.insert(result.decodedBytes.end(),decoded->data.begin(),decoded->data.end());
 result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(position),std::uint64_t(decoded->endOffset-position),Decoding::message(five?DataThirteenKey:DataKey,{literal(std::to_string(addr?addr->cylinder:0)),literal(std::to_string(LogicalHead)),literal(std::to_string(addr?addr->sector:0)),literal(std::to_string(SectorSize)),state(decoded->valid)})});
 return {std::move(decoded->data),decoded->valid,decoded->endOffset};
}
Decoding::Result core(const Flux::Bitstream& original,std::stop_token token){
 checkCancellation(token);const auto length=std::int64_t(original.bits().size());const auto stream=original.withCircularTail(CircularTailBitCount);const auto bits=stream.bits();
 Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();std::set<std::int64_t> paired;
 for(const bool five:{false,true}){
  for(std::int64_t offset=0;offset<length&&offset+PrologueBitCount<=std::int64_t(bits.size());++offset){
   checkCancellation(token);if(!Flux::match(bits,offset,five?FiveAndThreeAddressPrologue:SixAndTwoAddressPrologue,PrologueBitCount))continue;
   const auto addr=address(bits,offset,result,!five);const auto headerEnd=offset+(five?AddressBlockBitCount:PrologueBitCount+(addr?EncodedAddressBitCount:0));
   auto data=findData(bits,headerEnd,addr,five,result,paired,token);
   Decoding::Sector sector;sector.cylinder=addr?addr->cylinder:0;sector.head=LogicalHead;sector.number=addr?addr->sector:0;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorSize;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
   if((addr&&!addr->valid)||(data.valid&&!*data.valid))sector.integrityValid=false;else if(data.valid)sector.integrityValid=true;
   sector.data=std::move(data.payload);
   std::vector<Decoding::LocalizedText> args{literal(std::to_string(sector.cylinder)),literal(std::to_string(LogicalHead)),literal(std::to_string(sector.number)),literal(std::to_string(SectorSize)),literal(std::to_string(addr?addr->volume:0)),state(addr?std::optional(addr->valid):std::nullopt)};
   if(!five)args.push_back(state(data.valid));
   result.structures.push_back({Decoding::StructureKind::AppleAddress,std::uint64_t(offset),std::uint64_t(five?AddressBlockBitCount:std::max(std::int64_t(PrologueBitCount),headerEnd-offset)),Decoding::message(five?AddressThirteenKey:AddressKey,std::move(args))});
   result.sectors.push_back(std::move(sector));offset=addr&&addr->valid?std::max(offset+PrologueAdvanceBitCount,data.end-1):offset+PrologueAdvanceBitCount;
  }
 }
 for(std::int64_t offset=0;offset<length&&offset+PrologueBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!Flux::match(bits,offset,DataPrologue,PrologueBitCount)||paired.contains(offset))continue;result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(offset),PrologueBitCount,Decoding::message(UnpairedKey)});offset+=PrologueAdvanceBitCount;}
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){checkCancellation(token);return core(Flux::decodeNrzi(revolution.intervals,{},token),token);}
Decoding::Result decodeBits(Flux::BitView bits,std::stop_token token){checkCancellation(token);return core(Flux::Bitstream(Flux::Bits(bits.begin(),bits.end()),1),token);}
}
