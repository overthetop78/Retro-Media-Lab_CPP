#include "Flux.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Flux {
Bitstream::Bitstream(Bits bits,double cell):bits_(std::move(bits)),cell_(cell){
 if(!std::isfinite(cell)||cell<Constants::MinimumCell)throw MediaError(DiagnosticCode::FluxInvalidCell,{std::to_string(cell)});
 if(bits_.size()>Constants::MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
 if(std::any_of(bits_.begin(),bits_.end(),[](auto bit){return bit>1;}))throw MediaError(DiagnosticCode::FluxInvalidBit);
}
Bitstream Bitstream::withCircularTail(std::int64_t count)const {
 if(bits_.empty()||count<=0)return *this;const auto length=std::min(std::uint64_t(count),std::uint64_t(bits_.size()));
 if(length>Constants::MaximumBits-bits_.size())throw MediaError(DiagnosticCode::FluxSizeLimit);
 Bits result=bits_;result.insert(result.end(),bits_.begin(),bits_.begin()+std::size_t(length));return {std::move(result),cell_};
}
}
