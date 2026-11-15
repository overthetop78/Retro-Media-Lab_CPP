#include "Internal.h"
#include "../AppleII/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleIwm {
std::vector<std::uint8_t> encode(MediaFileSystems::Bytes source,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(source.size()!=TaggedSectorByteCount)throw MediaError(DiagnosticCode::IwmTaggedSize,{std::to_string(source.size())});
 Internal::Group first{},second{},third{};Internal::Accumulator state;std::size_t position=0;
 for(std::int32_t index=0;;++index){
  checkCancellation(token);state.rotateFirst();auto value=source[position++];first[index]=std::uint8_t(value^state.first);state.addFirst(value);
  value=source[position++];second[index]=std::uint8_t(value^state.third);state.addSecond(value);if(position==source.size())break;
  value=source[position++];third[index]=std::uint8_t(value^state.second);state.addThird(value);
 }
 std::vector<std::uint8_t> symbols;symbols.reserve(EncodedPayloadSymbolCount+ChecksumSymbolCount);
 for(std::int32_t index=0;index<=LastGroupIndex;++index){
  checkCancellation(token);symbols.push_back(std::uint8_t(((first[index]>>ThirdChecksumShift)&FirstPackedChecksumMask)|((second[index]>>SecondChecksumShift)&SecondPackedChecksumMask)|((third[index]>>FirstChecksumShift)&ThirdPackedChecksumMask)));
  symbols.push_back(first[index]&SixBitMask);symbols.push_back(second[index]&SixBitMask);if(index!=LastGroupIndex)symbols.push_back(third[index]&SixBitMask);
 }
 symbols.push_back(state.packed());symbols.push_back(std::uint8_t(state.third&SixBitMask));symbols.push_back(std::uint8_t(state.second&SixBitMask));symbols.push_back(std::uint8_t(state.first&SixBitMask));
 for(auto& value:symbols)value=AppleII::Constants::SixAndTwoTable[value];return symbols;
}
}
