#include "Flux.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <bit>
namespace RetroMediaLab::MediaEngine::Flux {
using namespace Constants;
namespace {
void validate(Intervals intervals,double cell,std::int32_t maximum,bool requirePositive = true){
 if(!std::isfinite(cell)||cell<=0)throw MediaError(DiagnosticCode::FluxInvalidCell,{std::to_string(cell)});
 if(requirePositive&&maximum<1)throw MediaError(DiagnosticCode::FluxInvalidMaximum,{std::to_string(maximum)});
 if(intervals.size()>MaximumBits/EstimatedBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
}
void append(Bits& bits,std::int32_t cells){if(std::uint64_t(cells)>MaximumBits-bits.size())throw MediaError(DiagnosticCode::FluxSizeLimit);bits.insert(bits.end(),std::size_t(cells)-1,0);bits.push_back(1);}
std::int32_t roundedCells(double ratio,std::int32_t maximum){
 const auto lower=std::floor(ratio),fraction=ratio-lower;
 const auto rounded=fraction>HalfCycle?lower+1:fraction<HalfCycle?lower:std::fmod(lower,RobustDivisor)==0?lower:lower+1;
 if(std::isnan(rounded)||rounded<=std::numeric_limits<std::int32_t>::min())return 1;
 if(rounded>=std::numeric_limits<std::int32_t>::max())return maximum;
 return std::clamp(std::int32_t(rounded),1,maximum);
}
double fixedCell(double cell){if(!std::isfinite(cell))throw MediaError(DiagnosticCode::FluxInvalidCell,{std::to_string(cell)});return std::max(MinimumCell,cell);}
}
Bitstream reconstruct(Intervals intervals,double initial,std::int32_t maximum,bool adapt,std::stop_token token){
 checkCancellation(token);validate(intervals,initial,maximum);auto current=initial;double accumulated=0;std::uint64_t samples=0;Bits bits;bits.reserve(intervals.size()*EstimatedBits);
 for(std::size_t index=0;index<intervals.size();++index){checkCancellation(token);const auto interval=intervals[index];const auto cells=roundedCells(interval/current,maximum);append(bits,cells);if(index==0||!adapt)continue;
  const auto observed=interval/double(cells);if(observed>=current*MinimumSampleRatio&&observed<=current*MaximumSampleRatio)current+=(observed-current)*Adaptation;
  accumulated+=current;++samples;
 }
 return {std::move(bits),samples?accumulated/samples:initial};
}
Bitstream reconstructPll(Intervals intervals,double centre,std::int32_t maximumCells,std::stop_token token){
 checkCancellation(token);validate(intervals,centre,maximumCells,false);const auto zeroLimit=std::bit_cast<std::int32_t>(std::uint32_t(maximumCells)-1);auto clock=centre;const auto minimum=centre*MinimumPllRatio,maximum=centre*MaximumPllRatio;double ticks=0,accumulated=0;std::uint64_t samples=0;Bits bits;bits.reserve(intervals.size()*EstimatedBits);
 for(const auto interval:intervals){checkCancellation(token);ticks+=interval;if(ticks<clock*HalfCycle)continue;std::int32_t zeros=0;
  while(zeros<zeroLimit){checkCancellation(token);ticks-=clock;if(ticks<clock*HalfCycle)break;++zeros;if(bits.size()>=MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);bits.push_back(0);}
  if(bits.size()>=MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);bits.push_back(1);
  const auto corrected=ticks*PhaseRetention;if(zeros<=MaximumDirectZeros)clock+=ticks*PllCorrection;else clock+=(centre-clock)*PllCorrection;
  clock=std::clamp(clock,minimum,maximum);ticks=corrected;accumulated+=clock;++samples;
 }
 return {std::move(bits),samples?accumulated/samples:centre};
}
Bitstream decodeAdaptiveFm(Intervals i,std::stop_token t){return reconstruct(i,estimateCell(i,TimingMode::Fm,t),MaximumFmMfmCells,true,t);}
Bitstream decodeAdaptiveMfm(Intervals i,std::stop_token t){return reconstruct(i,estimateCell(i,TimingMode::NonFm,t),MaximumFmMfmCells,true,t);}
Bitstream decodePllFm(Intervals i,std::stop_token t){return reconstructPll(i,estimateCell(i,TimingMode::Fm,t),MaximumFmMfmCells,t);}
Bitstream decodePllMfm(Intervals i,std::stop_token t){return reconstructPll(i,estimateCell(i,TimingMode::NonFm,t),MaximumFmMfmCells,t);}
Bitstream decodePll(Intervals i,double cell,std::stop_token t){return reconstructPll(i,fixedCell(cell),MaximumFmMfmCells,t);}
Bitstream decodeNrzi(Intervals i,std::optional<double> cell,std::stop_token t){return reconstruct(i,cell?fixedCell(*cell):estimateCell(i,TimingMode::Nrzi,t),MaximumNrziCells,!cell,t);}
Bitstream decodeAdaptiveDoubledNrzi(Intervals i,std::stop_token t){return reconstruct(i,estimateCell(i,TimingMode::NonFm,t),MaximumNrziCells,true,t);}
}
