#include "Gcr.h"
#include "../AppleII/Constants.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleIwm {
namespace {
void validate(std::string_view field,std::int32_t value,std::int32_t maximum){if(value<0||value>maximum)throw MediaError(DiagnosticCode::IwmAddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validate(Encoding::Constants::CylinderField,request.cylinder,MaximumCylinder);validate(Encoding::Constants::HeadField,request.head,MaximumHead);
 const auto format=attribute(request.attributes,std::string(FormatAttribute),DefaultFormat);validate(FormatField,format,SixBitMask);
 Flux::Bits bits;const auto& table=AppleII::Constants::SixAndTwoTable;
 for(const auto& sector:request.sectors){
  checkCancellation(token);if(sector.data.size()!=SectorByteCount)throw MediaError(DiagnosticCode::IwmSectorSize,{std::to_string(sector.data.size())});validate(Encoding::Constants::NumberField,sector.number,SixBitMask);
  const std::array<std::uint8_t,HeaderValueCount> header{std::uint8_t(request.cylinder&SixBitMask),std::uint8_t(sector.number),std::uint8_t(((request.cylinder>>CylinderHighBitShift)&CylinderHighBitMask)|(request.head<<HeadBitShift)),std::uint8_t(format)};
  std::uint8_t checksum=0;Encoding::gap(bits,AddressLeadingGapBitCount,true);Encoding::raw(bits,AddressMark);
  std::array<std::uint8_t,HeaderSymbolCount> encodedHeader{};for(std::size_t i=0;i<header.size();++i){checksum^=header[i];encodedHeader[i]=table[header[i]];}encodedHeader[HeaderChecksumOffset]=table[checksum&SixBitMask];Encoding::raw(bits,encodedHeader);Encoding::raw(bits,AddressEpilogue);Encoding::gap(bits,AddressTrailingGapBitCount,true);
  std::vector<std::uint8_t> tagged;tagged.reserve(TaggedSectorByteCount);
  for(std::int32_t i=0;i<TagByteCount;++i){const auto key=std::string(TagPrefix)+std::to_string(i);const auto value=attribute(sector.attributes,key,0);validate(key,value,MaximumTagValue);tagged.push_back(std::uint8_t(value));}
  tagged.insert(tagged.end(),sector.data.begin(),sector.data.end());Encoding::raw(bits,DataMark);const std::array<std::uint8_t,1> number{table[sector.number]};Encoding::raw(bits,number);Encoding::raw(bits,AppleIwm::encode(tagged,token));Encoding::raw(bits,DataEpilogue);Encoding::gap(bits,DataTrailingGapBitCount,true);
 }
 return bits;
}
}
