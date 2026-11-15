#include "Rx02.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DecRx02 {
namespace {
void replace(Flux::Bits& bits,Flux::BitView source,Flux::BitView target,std::int64_t start,std::int64_t step,std::int64_t skip,std::stop_token token){
 for(auto offset=start;offset+std::int64_t(source.size())<=std::int64_t(bits.size());offset+=step){checkCancellation(token);if(offset%Constants::CellsPerBit!=start%Constants::CellsPerBit||!std::equal(source.begin(),source.end(),bits.begin()+offset))continue;std::copy(target.begin(),target.end(),bits.begin()+offset);offset+=skip;}
}
}
void encodeM2Fm(Flux::Bits& bits,std::stop_token token){
 checkCancellation(token);if(bits.size()>Constants::MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);for(auto bit:bits){checkCancellation(token);if(bit>1)throw MediaError(DiagnosticCode::FluxInvalidBit);}replace(bits,Constants::NormalRule,Constants::EncodedRule,Constants::EncodeStart,Constants::EncodeStep,Constants::EncodeSkip,token);
}
std::optional<std::vector<std::uint8_t>> decodeM2Fm(Flux::BitView stream,std::int64_t start,std::int64_t count,std::stop_token token){
 checkCancellation(token);if(count<0||std::uint64_t(count)>Constants::MaximumDecodedBytes)throw MediaError(DiagnosticCode::BitReadCount,{std::to_string(count),std::to_string(Constants::MaximumDecodedBytes)});
 const auto required=count*Constants::EncodedMfmByteBitCount;if(start>std::int64_t(stream.size())-required)return {};if(required==0)return std::vector<std::uint8_t>{};if(start<0)throw MediaError(DiagnosticCode::Rx02M2FmNegativeStart,{std::to_string(start)});
 Flux::Bits bits(std::size_t(required)+Constants::PhaseBitCount);for(std::int64_t index=0;index<required;++index){checkCancellation(token);const auto bit=stream[std::size_t(start+index)];if(bit>1)throw MediaError(DiagnosticCode::FluxInvalidBit);bits[std::size_t(index)+Constants::PhaseBitCount]=bit;}
 replace(bits,Constants::EncodedRule,Constants::NormalRule,Constants::DecodeStart,Constants::DecodeStep,Constants::DecodeSkip,token);std::vector<std::uint8_t> result(std::size_t(count),0);
 for(std::int64_t index=0;index<count;++index){checkCancellation(token);for(int bit=0;bit<Constants::BitsPerByte;++bit){const auto position=Constants::PhaseBitCount+index*Constants::EncodedMfmByteBitCount+bit*Constants::CellsPerBit;if(bits[std::size_t(position)]&&bits[std::size_t(position)+1])return {};if(!bits[std::size_t(position)]&&bits[std::size_t(position)+1])result[std::size_t(index)]|=std::uint8_t(1<<(Constants::BitsPerByte-1-bit));}}
 return result;
}
}
