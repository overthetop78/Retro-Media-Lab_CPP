#include "Codec.h"
#include "../AppleII/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
namespace RetroMediaLab::MediaEngine::AppleRwts18 {
std::vector<std::uint8_t> encodePayload(MediaFileSystems::Bytes data,std::stop_token token){
 using namespace Constants;checkCancellation(token);
 if(data.size()!=SectorByteCount)throw MediaError(DiagnosticCode::Rwts18SectorSize,{std::to_string(PayloadSectorNumber),std::to_string(data.size())});
 std::vector<std::uint8_t> encoded(PayloadWithChecksumSymbolCount);std::uint8_t checksum=0;
 for(std::int32_t index=0;index<PageByteCount;++index){
  checkCancellation(token);const auto one=data[index],two=data[PageByteCount*SecondPageIndex+index],three=data[PageByteCount*ThirdPageIndex+index];
  const auto high=std::uint8_t(((one>>SourceHighBitShift)<<FirstPagePackedShift)|((two>>SourceHighBitShift)<<SecondPagePackedShift)|(three>>SourceHighBitShift));
  const std::array<std::uint8_t,SymbolsPerPageGroup> values{high,std::uint8_t(one&SixBitMask),std::uint8_t(two&SixBitMask),std::uint8_t(three&SixBitMask)};
  for(std::int32_t valueIndex=0;valueIndex<SymbolsPerPageGroup;++valueIndex){const auto value=values[valueIndex];checksum^=value;encoded[index*SymbolsPerPageGroup+valueIndex]=AppleII::Constants::SixAndTwoTable[value];}
 }
 encoded[PayloadSymbolCount]=AppleII::Constants::SixAndTwoTable[checksum&SixBitMask];return encoded;
}
DecodedPayload decodePayload(MediaFileSystems::Bytes values,std::stop_token token){
 using namespace Constants;checkCancellation(token);
 if(values.size()<PayloadWithChecksumSymbolCount)throw MediaError(DiagnosticCode::Rwts18PayloadLength,{std::to_string(values.size()),std::to_string(PayloadWithChecksumSymbolCount)});
 std::vector<std::uint8_t> data(SectorByteCount);std::uint8_t accumulator=0,previousPage1=0;
 for(std::int32_t index=0;index<PageByteCount;++index){
  checkCancellation(token);const auto base=index*SymbolsPerPageGroup;const auto high=values[base+HighSymbolOffset];const auto checksum=std::uint8_t(accumulator^previousPage1^high);
  data[index]=std::uint8_t(((high<<FirstPageHighBitShift)&HighBitMask)|values[base+FirstPageSymbolOffset]);previousPage1=data[index];
  data[PageByteCount*SecondPageIndex+index]=std::uint8_t(((high<<SecondPageHighBitShift)&HighBitMask)|values[base+SecondPageSymbolOffset]);
  data[PageByteCount*ThirdPageIndex+index]=std::uint8_t(((high<<ThirdPageHighBitShift)&HighBitMask)|values[base+ThirdPageSymbolOffset]);
  accumulator=std::uint8_t(data[PageByteCount*ThirdPageIndex+index]^data[PageByteCount*SecondPageIndex+index]^checksum);
 }
 const bool valid=((accumulator^values[PayloadSymbolCount]^previousPage1)&SixBitMask)==0;return {std::move(data),valid};
}
}
