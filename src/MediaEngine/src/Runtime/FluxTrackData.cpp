#include <RetroMediaLab/MediaEngine/FluxTrackData.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "FluxConstants.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace C=RuntimeDetails::FluxConstants;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeTrackData);}
void put(std::vector<std::uint8_t>& out,std::uint32_t value){for(std::size_t i=0;i<C::UInt32Size;++i)out.push_back(static_cast<std::uint8_t>((value>>(i*C::ByteBits))&C::ByteMask));}
std::uint32_t get(std::span<const std::uint8_t> bytes,std::size_t& pos){if(pos>bytes.size()||bytes.size()-pos<C::UInt32Size)invalid();std::uint32_t result=0;for(std::size_t i=0;i<C::UInt32Size;++i)result|=std::uint32_t(bytes[pos++])<<(i*C::ByteBits);return result;}
}
FluxRevolutionData::FluxRevolutionData(std::uint32_t period,std::vector<std::uint32_t> values):period_(period),intervals_(std::move(values)){if(!period_||std::find(intervals_.begin(),intervals_.end(),0)!=intervals_.end())invalid();}
FluxTrackData::FluxTrackData(std::vector<FluxRevolutionData> values):revolutions_(std::move(values)){if(revolutions_.empty())invalid();}
FluxRevolutionData replaceFluxWindow(const FluxRevolutionData& source,std::uint32_t startNanoseconds,std::uint32_t duration,std::span<const std::uint32_t> offsets,bool ending){
    std::uint32_t previous=0;bool first=true;for(auto offset:offsets){if(offset>duration||(!ending&&offset==duration)||(!first&&offset<=previous))throw MediaError(DiagnosticCode::RuntimeTrackWindow);previous=offset;first=false;}
    const auto period=source.indexTimeNanoseconds();const auto start=startNanoseconds%period;std::set<std::uint32_t> transitions;std::uint64_t position=0;
    for(auto interval:source.intervalsNanoseconds()){
        position+=interval;if(position>period)invalid();const auto relative=(position%period+period-start)%period;
        if(relative>duration||(!ending&&relative==duration))transitions.insert(static_cast<std::uint32_t>(position));
    }
    for(auto offset:offsets){const auto next=std::uint64_t(offset)+period;if(next<duration||(ending&&next==duration))continue;const auto phase=static_cast<std::uint32_t>((std::uint64_t(start)+offset)%period);transitions.insert(phase?phase:period);}
    std::vector<std::uint32_t> values;values.reserve(transitions.size());previous=0;for(auto transition:transitions){values.push_back(transition-previous);previous=transition;}return {period,std::move(values)};
}
std::vector<std::uint8_t> serializeFluxTrack(const FluxTrackData& track){
    if(track.revolutions().size()>C::MaximumRevolutions)invalid();std::uint64_t length=C::HeaderSize;
    for(const auto& r:track.revolutions()){if(r.intervalsNanoseconds().size()>C::MaximumIntervals)invalid();length+=C::RevolutionHeaderSize+r.intervalsNanoseconds().size()*C::UInt32Size;if(length>C::MaximumPayload)invalid();}
    std::vector<std::uint8_t> result;result.reserve(static_cast<std::size_t>(length));put(result,C::Magic);put(result,static_cast<std::uint32_t>(track.revolutions().size()));
    for(const auto& r:track.revolutions()){put(result,r.indexTimeNanoseconds());put(result,static_cast<std::uint32_t>(r.intervalsNanoseconds().size()));for(auto interval:r.intervalsNanoseconds())put(result,interval);}return result;
}
FluxTrackData deserializeFluxTrack(std::span<const std::uint8_t> bytes){
    if(bytes.size()>C::MaximumPayload)invalid();std::size_t pos=0;if(get(bytes,pos)!=C::Magic)invalid();const auto count=get(bytes,pos);if(!count||count>C::MaximumRevolutions)invalid();std::vector<FluxRevolutionData> result;result.reserve(count);
    for(std::uint32_t i=0;i<count;++i){const auto period=get(bytes,pos),size=get(bytes,pos);if(size>C::MaximumIntervals||std::uint64_t(size)*C::UInt32Size>bytes.size()-pos)invalid();std::vector<std::uint32_t> values;values.reserve(size);for(std::uint32_t j=0;j<size;++j)values.push_back(get(bytes,pos));result.emplace_back(period,std::move(values));}
    if(pos!=bytes.size())invalid();return FluxTrackData(std::move(result));
}
}
