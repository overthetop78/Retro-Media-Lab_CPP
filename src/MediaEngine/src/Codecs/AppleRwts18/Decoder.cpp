#include "Gcr.h"
#include "../Apple/BitLatch.h"
#include "../AppleII/Internal.h"
#include "../../Models/Flux/BitReader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AppleRwts18 {
namespace {
using namespace Constants;
struct Address {std::uint8_t track,sector;std::int64_t cursor;};
struct Data {DecodedPayload payload;std::int64_t start,end;};
std::optional<Address> readAddress(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 auto cursor=offset+AddressMarkBitCount;const auto address=Apple::readBytes(bits,cursor,AddressByteCount,token);if(!address)return {};
 const auto& inverse=AppleII::Internal::InverseSixAndTwoTable;
 const auto track=inverse[(*address)[AddressTrackOffset]],sector=inverse[(*address)[AddressSectorOffset]],checksum=inverse[(*address)[AddressChecksumOffset]];
 if(track<0||sector<0||checksum<0||(*address)[AddressTrailerOffset]!=AddressTrailer||sector>=SectorCount||(track^sector)!=checksum)return {};
 return Address{std::uint8_t(track),std::uint8_t(sector),cursor};
}
std::optional<Data> readData(Flux::BitView bits,std::int64_t offset,std::stop_token token){
 auto cursor=offset;const auto stream=Apple::readBytes(bits,cursor,DataReadWindowByteCount,token);if(!stream)return {};
 const auto& inverse=AppleII::Internal::InverseSixAndTwoTable;
 for(std::int32_t start=0;start+DataRecordByteCount<=std::int32_t(stream->size());++start){
  checkCancellation(token);std::vector<std::uint8_t> values(PayloadWithChecksumSymbolCount);bool valid=true;
  for(std::int32_t index=0;index<PayloadWithChecksumSymbolCount;++index){const auto value=inverse[(*stream)[start+PayloadOffset+index]];if(value<0){valid=false;break;}values[index]=std::uint8_t(value);}
  if(!valid||(*stream)[start+DataEpilogueOffset]!=DataEpilogue)continue;
  return Data{decodePayload(values,token),offset+start*BitsPerByte,offset+(start+DataRecordByteCount)*BitsPerByte};
 }
 return {};
}
Decoding::Result core(const Flux::Bitstream& source,std::stop_token token){
 checkCancellation(token);const auto length=std::int64_t(source.bits().size());const auto stream=source.withCircularTail(CircularTailBitCount);const auto bits=stream.bits();
 Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=source.cellTicks();std::size_t valid=0;
 for(std::int64_t offset=0;offset+AddressMarkBitCount<=length;++offset){
  checkCancellation(token);if(!Flux::match(bits,offset,EncodedAddressMark,AddressMarkBitCount))continue;
  const auto address=readAddress(bits,offset,token);if(!address)continue;auto data=readData(bits,address->cursor,token);
  using Decoding::literal;
  const auto args=std::vector<Decoding::LocalizedText>{literal(std::to_string(address->track)),literal(std::to_string(LogicalHead)),literal(std::to_string(address->sector)),literal(std::to_string(SectorByteCount))};
  auto addressArgs=args;addressArgs.push_back(Decoding::message(ValidKey));result.structures.push_back({Decoding::StructureKind::AppleAddress,std::uint64_t(offset),std::uint64_t(address->cursor-offset),Decoding::message(AddressKey,std::move(addressArgs))});
  Decoding::Sector sector;sector.cylinder=address->track;sector.head=LogicalHead;sector.number=address->sector;sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorByteCount;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
  if(data){auto dataArgs=args;dataArgs.push_back(Decoding::message(data->payload.valid?ValidKey:InvalidKey));result.structures.push_back({Decoding::StructureKind::AppleData,std::uint64_t(data->start),std::uint64_t(data->end-data->start),Decoding::message(DataKey,std::move(dataArgs))});result.decodedBytes.insert(result.decodedBytes.end(),data->payload.data.begin(),data->payload.data.end());sector.integrityValid=data->payload.valid;if(data->payload.valid)++valid;sector.data=std::move(data->payload.data);}
  result.sectors.push_back(std::move(sector));offset=std::max(offset+AddressMarkAdvanceBitCount,(data?data->end:address->cursor)-1);
 }
 result.confidence=result.sectors.empty()?0:std::min(MaximumConfidence,valid/ConfidenceCompleteSectorDivisor+result.sectors.size()/ConfidenceDetectedSectorDivisor);return result;
}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){checkCancellation(token);return core(Flux::decodeNrzi(revolution.intervals,{},token),token);}
Decoding::Result decodeBits(Flux::BitView bits,std::stop_token token){checkCancellation(token);return core(Flux::Bitstream(Flux::Bits(bits.begin(),bits.end()),1),token);}
}
