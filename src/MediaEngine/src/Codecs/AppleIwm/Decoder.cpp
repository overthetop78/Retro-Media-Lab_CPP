#include "Gcr.h"
#include "../AppleII/Internal.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine::AppleIwm {
namespace {
using namespace Constants;
struct Address {std::uint8_t cylinder,head,sector,format;bool valid;};
struct Data {std::optional<std::vector<std::uint8_t>> data,tag;std::optional<bool> valid;std::int64_t end;};
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
std::optional<std::vector<std::uint8_t>> symbols(Flux::BitView bits,std::int64_t offset,std::int32_t count,std::stop_token token){
 if(offset+count*BitsPerByte>std::int64_t(bits.size()))return {};std::vector<std::uint8_t> result;result.reserve(count);
 for(std::int32_t i=0;i<count;++i){checkCancellation(token);const auto byte=Flux::rawByte(bits,offset+i*BitsPerByte);if(!byte)return {};result.push_back(*byte);}return result;
}
std::optional<std::vector<std::uint8_t>> decodeSymbols(const std::optional<std::vector<std::uint8_t>>& encoded,std::stop_token token){
 if(!encoded)return {};auto result=*encoded;const auto& inverse=AppleII::Internal::InverseSixAndTwoTable;
 for(auto& symbol:result){checkCancellation(token);const auto value=inverse[symbol];if(value<0)return {};symbol=std::uint8_t(value);}return result;
}
std::optional<Address> address(const std::optional<std::vector<std::uint8_t>>& header){
 if(!header)return {};const auto& v=*header;const auto cylinder=std::uint8_t(((v[HeaderHeadOffset]&CylinderHighBitMask)<<CylinderHighBitShift)|(v[HeaderCylinderOffset]&SixBitMask));const auto head=std::uint8_t((v[HeaderHeadOffset]>>HeadBitShift)&HeadBitMask);
 const bool valid=((v[HeaderCylinderOffset]^v[HeaderSectorOffset]^v[HeaderHeadOffset]^v[HeaderFormatOffset])&SixBitMask)==v[HeaderChecksumOffset];return Address{cylinder,head,v[HeaderSectorOffset],v[HeaderFormatOffset],valid};
}
Data findData(Flux::BitView bits,std::int64_t headerEnd,const std::optional<Address>& addr,Decoding::Result& result,std::set<std::int64_t>& paired,std::stop_token token){
 std::int64_t position=-1;const auto end=std::min(std::int64_t(bits.size()),headerEnd+DataSearchBitCount);
 if(addr&&addr->valid)for(auto offset=headerEnd;offset+MarkBitCount<=end;++offset){checkCancellation(token);if(Flux::matchBytes(bits,offset,DataMark)){position=offset;break;}}
 if(position<0)return {{},{},{},headerEnd};paired.insert(position);auto values=decodeSymbols(symbols(bits,position+MarkBitCount,DataSymbolCount,token),token);
 if(!values){result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(position),MarkBitCount,Decoding::message(TruncatedKey)});return {{},{},{},headerEnd};}
 const auto decoded=AppleIwm::decode(MediaFileSystems::Bytes(*values).subspan(DataPayloadOffset,EncodedPayloadSymbolCount),token);const auto& checksum=decoded.checksum;
 const bool valid=checksum[ChecksumPackedOffset]==(*values)[PackedChecksumSymbolOffset]&&checksum[ChecksumThirdOffset]==(*values)[ThirdChecksumSymbolOffset]&&checksum[ChecksumSecondOffset]==(*values)[SecondChecksumSymbolOffset]&&checksum[ChecksumFirstOffset]==(*values)[FirstChecksumSymbolOffset];
 std::vector<std::uint8_t> tag(decoded.data.begin(),decoded.data.begin()+TagByteCount),data(decoded.data.begin()+TagByteCount,decoded.data.end());const auto structureEnd=position+MarkBitCount+DataSymbolCount*BitsPerByte;result.decodedBytes.insert(result.decodedBytes.end(),data.begin(),data.end());
 using Decoding::literal;result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(position),std::uint64_t(structureEnd-position),Decoding::message(DataKey,{literal(std::to_string(addr->cylinder)),literal(std::to_string(addr->head)),literal(std::to_string(addr->sector)),literal(std::to_string(SectorByteCount)),state(valid)})});return {std::move(data),std::move(tag),valid,structureEnd};
}
Decoding::Result core(const Flux::Bitstream& original,Variant variant,std::stop_token token){
 checkCancellation(token);const auto length=std::int64_t(original.bits().size());const auto stream=original.withCircularTail(CircularTailBitCount);const auto bits=stream.bits();Decoding::Result result;result.decoderId=codecId(variant);result.displayName=codecName(variant);result.estimatedCellTicks=stream.cellTicks();std::set<std::int64_t> paired;
 for(std::int64_t offset=0;offset<length&&offset+MarkBitCount<=std::int64_t(bits.size());++offset){
  checkCancellation(token);if(!Flux::matchBytes(bits,offset,AddressMark))continue;
  const auto header=symbols(bits,offset+MarkBitCount,HeaderSymbolCount,token);const auto addr=address(decodeSymbols(header,token));const auto headerEnd=offset+MarkBitCount+(header?HeaderSymbolCount*BitsPerByte:0);auto data=findData(bits,headerEnd,addr,result,paired,token);
  Decoding::Sector sector;sector.cylinder=addr?addr->cylinder:0;sector.head=addr?addr->head:0;sector.number=addr?addr->sector:0;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorByteCount;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
  if((addr&&!addr->valid)||(data.valid&&!*data.valid))sector.integrityValid=false;else if(data.valid)sector.integrityValid=true;sector.data=std::move(data.data);sector.tag=std::move(data.tag);if(addr)sector.formatCode=addr->format;
  using Decoding::literal;result.structures.push_back({Decoding::StructureKind::AppleAddress,std::uint64_t(offset),std::uint64_t(std::max(std::int64_t(MarkBitCount),headerEnd-offset)),Decoding::message(AddressKey,{literal(std::to_string(sector.cylinder)),literal(std::to_string(sector.head)),literal(std::to_string(sector.number)),literal(std::to_string(SectorByteCount)),state(addr?std::optional(addr->valid):std::nullopt),state(data.valid)})});result.sectors.push_back(std::move(sector));offset=addr&&addr->valid?std::max(offset+MarkAdvanceBitCount,data.end-1):offset+MarkAdvanceBitCount;
 }
 for(std::int64_t offset=0;offset<length&&offset+MarkBitCount<=std::int64_t(bits.size());++offset){checkCancellation(token);if(!Flux::matchBytes(bits,offset,DataMark)||paired.contains(offset))continue;result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(offset),MarkBitCount,Decoding::message(UnpairedKey)});offset+=MarkAdvanceBitCount;}
 result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);std::stable_sort(result.structures.begin(),result.structures.end(),[](const auto& a,const auto& b){return a.bitOffset<b.bitOffset;});return result;
}
}
Decoding::Result decode(const Flux::Revolution& revolution,Variant variant,std::stop_token token){checkCancellation(token);return core(Flux::decodeNrzi(revolution.intervals,{},token),variant,token);}
Decoding::Result decodeBits(Flux::BitView bits,Variant variant,std::stop_token token){checkCancellation(token);return core(Flux::Bitstream(Flux::Bits(bits.begin(),bits.end()),1),variant,token);}
Decoding::Result decodeAtBitCell(const Flux::Revolution& revolution,double cell,Variant variant,std::stop_token token){checkCancellation(token);return core(Flux::decodeNrzi(revolution.intervals,cell,token),variant,token);}
}
