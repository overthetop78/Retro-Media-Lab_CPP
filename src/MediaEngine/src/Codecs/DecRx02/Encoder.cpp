#include "Rx02.h"
#include "../Crc16.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::DecRx02 {
namespace {
void validateAddress(std::string_view field,std::int32_t value){if(value<0||value>Constants::MaximumAddress)throw MediaError(DiagnosticCode::Rx02AddressRange,{std::string(field),std::to_string(value),std::to_string(Constants::MaximumAddress)});}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validateAddress(Encoding::Constants::CylinderField,request.cylinder);validateAddress(Encoding::Constants::HeadField,request.head);Flux::Bits bits;
 for(const auto& sector:request.sectors){
  checkCancellation(token);validateAddress(Encoding::Constants::NumberField,sector.number);if(sector.data.size()!=FmSectorSize&&sector.data.size()!=M2FmSectorSize)throw MediaError(DiagnosticCode::Rx02SectorSize,{std::to_string(sector.data.size())});const auto format=encodingForSize(std::int32_t(sector.data.size()));
  if(sector.sizeCode&&*sector.sizeCode!=format.sizeCode)throw MediaError(DiagnosticCode::Rx02SizeCode,{std::to_string(*sector.sizeCode),std::to_string(sector.data.size())});
  const std::array fields{HeaderAddressMark,std::uint8_t(request.cylinder),std::uint8_t(request.head),std::uint8_t(sector.number),format.sizeCode};
  const auto header=Crc16::append(fields,CrcPolynomial,CrcInitialValue,token);Encoding::raw(bits,FmAddressMarks::forMark(HeaderAddressMark));Encoding::doubleFm(bits,MediaFileSystems::Bytes(header).subspan(1));Encoding::gap(bits,GapBitCount,true);
  const auto& mark=dataMarkFor(format.encoding,sector.deleted);Encoding::raw(bits,mark.pattern);std::vector<std::uint8_t> data;data.reserve(sector.data.size()+1);data.push_back(mark.mark);data.insert(data.end(),sector.data.begin(),sector.data.end());const auto withCrc=Crc16::append(data,CrcPolynomial,CrcInitialValue,token);const auto payload=MediaFileSystems::Bytes(withCrc).subspan(1);
  if(mark.encoding==DataEncoding::M2Fm){bits.push_back(0);Flux::Bits encoded;Encoding::mfm(encoded,payload);encodeM2Fm(encoded,token);bits.insert(bits.end(),encoded.begin(),encoded.end());}else Encoding::doubleFm(bits,payload);
  Encoding::gap(bits,GapBitCount,true);
 }
 return bits;
}
}
