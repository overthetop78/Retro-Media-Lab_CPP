#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleIwm {
DecodedPayload decode(MediaFileSystems::Bytes symbols,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(symbols.size()<EncodedPayloadSymbolCount)throw MediaError(DiagnosticCode::IwmSymbolLength,{std::to_string(symbols.size()),std::to_string(EncodedPayloadSymbolCount)});
 Internal::Group first{},second{},third{};std::size_t source=0;
 for(std::int32_t index=0;index<=LastGroupIndex;++index){
  checkCancellation(token);const auto high=symbols[source++],one=symbols[source++],two=symbols[source++];const auto three=index==LastGroupIndex?std::uint8_t(0):symbols[source++];
  first[index]=std::uint8_t((one&SixBitMask)|((high<<ThirdChecksumShift)&EncodedHighBitsMask));
  second[index]=std::uint8_t((two&SixBitMask)|((high<<SecondChecksumShift)&EncodedHighBitsMask));
  third[index]=std::uint8_t((three&SixBitMask)|((high<<FirstChecksumShift)&EncodedHighBitsMask));
 }
 DecodedPayload output;output.data.resize(TaggedSectorByteCount);Internal::Accumulator state;std::size_t destination=0;
 for(std::int32_t index=0;;++index){
  checkCancellation(token);state.rotateFirst();auto value=std::uint8_t(first[index]^state.first);state.addFirst(value);output.data[destination++]=value;
  value=std::uint8_t(second[index]^state.third);state.addSecond(value);output.data[destination++]=value;if(destination==TaggedSectorByteCount)break;
  value=std::uint8_t(third[index]^state.second);state.addThird(value);output.data[destination++]=value;
 }
 output.checksum={std::uint8_t(state.first&SixBitMask),std::uint8_t(state.second&SixBitMask),std::uint8_t(state.third&SixBitMask),state.packed()};return output;
}
}
