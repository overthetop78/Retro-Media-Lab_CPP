#include "Fm.h"
#include "../../Models/Flux/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
namespace RetroMediaLab::MediaEngine::IsoFm {
namespace {
double roundEven(double value){const auto lower=std::floor(value),fraction=value-lower;return fraction>Flux::Constants::HalfCycle?lower+1:fraction<Flux::Constants::HalfCycle?lower:std::fmod(lower,Flux::Constants::RobustDivisor)==0?lower:lower+1;}
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){return decodeCore(Flux::decodeAdaptiveFm(revolution.intervals,token),token);}
Decoding::Result decode(const Flux::Revolution& revolution,double cell,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(!std::isfinite(cell)||cell<=0)throw MediaError(DiagnosticCode::FluxInvalidCell,{std::to_string(cell)});
    const auto source=Flux::reconstruct(revolution.intervals,cell,Flux::Constants::MaximumFmMfmCells,false,token);
    const auto rounded=roundEven(revolution.indexTimeTicks/cell);if(!std::isfinite(rounded)||rounded>MaximumPeriod)throw MediaError(DiagnosticCode::FluxSizeLimit);
    const auto period=std::int64_t(rounded);std::uint64_t observed=0;for(auto interval:revolution.intervals){checkCancellation(token);observed+=interval;}
    if(period<=0||observed>revolution.indexTimeTicks)throw MediaError(DiagnosticCode::FmRevolutionTiming,{std::to_string(revolution.indexTimeTicks),std::to_string(cell),std::to_string(observed)});
    const auto length=std::uint64_t(period)*CellsPerDataBit+MaximumDataBits;if(length>Flux::Constants::MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
    Flux::Bits bits(std::size_t(length),0);for(std::size_t offset=0;offset<bits.size();++offset){checkCancellation(token);const auto phase=offset%std::size_t(period);if(phase<source.bits().size())bits[offset]=source.bits()[phase];}
    return decodeCore(Flux::Bitstream(std::move(bits),cell),token,std::int32_t(period));
}
}
