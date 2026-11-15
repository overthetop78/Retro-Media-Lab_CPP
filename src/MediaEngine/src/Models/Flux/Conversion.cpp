#include "Conversion.h"
#include "ConversionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
#include <numeric>
namespace RetroMediaLab::MediaEngine::Flux {
namespace {
std::uint32_t checked(std::uint64_t value){if(value>ConversionConstants::MaximumUnsigned)throw MediaError(DiagnosticCode::FluxConversionRange);return std::uint32_t(value);}
std::uint32_t rounded(double value){if(!std::isfinite(value)||std::nearbyint(value)<0||std::nearbyint(value)>ConversionConstants::MaximumUnsigned)throw MediaError(DiagnosticCode::FluxConversionRange);return std::uint32_t(std::nearbyint(value));}
void validate(const NanosecondRevolution& r){if(!r.indexTime)throw MediaError(DiagnosticCode::FluxZeroIndex,{std::to_string(r.indexTime)});for(auto interval:r.intervals)if(!interval)throw MediaError(DiagnosticCode::FluxConversionInterval);}
}
std::uint32_t greatestCommonDivisor(Intervals values) noexcept{std::uint32_t result=0;for(auto value:values)result=std::gcd(result,value);return result;}
std::vector<std::uint32_t> toIntervals(BitView bits,std::uint32_t ticks){
 if(!ticks)throw MediaError(DiagnosticCode::FluxConversionTiming);std::vector<std::uint32_t> result;std::uint64_t cells=0;for(auto bit:bits){++cells;if(bit){result.push_back(checked(cells*ticks));cells=0;}}return result;
}
Bits toBits(Intervals intervals,std::uint32_t index,std::uint32_t ticks){
 if(!ticks||!index||index%ticks)throw MediaError(DiagnosticCode::FluxConversionTiming);const auto count=index/ticks;if(count>std::uint32_t(ConversionConstants::MaximumCells))throw MediaError(DiagnosticCode::FluxConversionRange);Bits result;result.reserve(count);
 for(auto interval:intervals){if(!interval||interval%ticks)throw MediaError(DiagnosticCode::FluxConversionTiming);const auto cells=interval/ticks;if(cells>count-result.size())throw MediaError(DiagnosticCode::FluxConversionBounds);result.insert(result.end(),cells-1,0);result.push_back(1);}result.resize(count,0);return result;
}
std::vector<NanosecondRevolution> readFlux(const ProtectedTrack& track){
 std::vector<NanosecondRevolution> result;
 if(!track.bits()){if(track.revolutions().empty())throw MediaError(DiagnosticCode::FluxConversionTiming);for(const auto& source:track.revolutions()){NanosecondRevolution r;r.indexTime=checked(std::uint64_t(source.flux().indexTimeTicks)*source.resolutionNanoseconds());for(auto interval:source.flux().intervals)r.intervals.push_back(checked(std::uint64_t(interval)*source.resolutionNanoseconds()));validate(r);result.push_back(std::move(r));}return result;}
 NanosecondRevolution r;double time=0;std::uint32_t previous=0;std::int64_t offset=0;
 for(const auto& segment:track.timing()){if(segment.bitOffset!=offset)throw MediaError(DiagnosticCode::FluxConversionBounds);const auto end=segment.bitOffset+segment.bitLength;if(end>std::int64_t(track.bits()->size()))throw MediaError(DiagnosticCode::FluxConversionBounds);for(;offset<end;++offset){time+=segment.cellNanoseconds;if(!(*track.bits())[std::size_t(offset)])continue;const auto position=rounded(time);if(position<previous)throw MediaError(DiagnosticCode::FluxConversionRange);r.intervals.push_back(position-previous);previous=position;}}
 if(offset!=std::int64_t(track.bits()->size()))throw MediaError(DiagnosticCode::FluxConversionBounds);r.indexTime=rounded(time);validate(r);result.push_back(std::move(r));return result;
}
}
