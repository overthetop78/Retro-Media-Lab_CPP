#include "Raw.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <bit>
#include <cmath>
namespace RetroMediaLab::MediaEngine::RawFlux {
std::int32_t convertToCellCount(std::uint32_t interval,double cell) noexcept {
    using namespace Constants;
    const auto ratio=interval/cell;
    if(std::isnan(ratio)||ratio<=MinimumCellCount)return MinimumCellCount;
    if(ratio>=MaximumCellCount)return MaximumCellCount;
    const auto lower=std::floor(ratio),fraction=ratio-lower;
    const auto rounded=fraction>Half?lower+1:fraction<Half?lower:std::fmod(lower,EvenDivisor)==0?lower:lower+1;
    return std::clamp(std::int32_t(rounded),MinimumCellCount,MaximumCellCount);
}
AnomalyKind classify(std::uint32_t interval,double cell,std::int32_t index) noexcept {
    if(interval>cell*Constants::LongIntervalMultiplier)return AnomalyKind::LongInterval;
    return index>0&&interval<cell*Constants::ShortPulseRatio?AnomalyKind::ShortPulse:AnomalyKind::None;
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
    checkCancellation(token);
    const auto cell=Flux::estimateCell(revolution.intervals,Flux::TimingMode::NonFm,token);
    Decoding::Result result;
    result.decoderId=Constants::Id;result.displayName=Constants::SourceDisplayName;
    result.confidence=Constants::Confidence;result.estimatedCellTicks=cell;
    std::uint32_t offset=0;
    for(std::size_t index=0;index<revolution.intervals.size();++index){
        checkCancellation(token);
        const auto interval=revolution.intervals[index];
        const auto count=convertToCellCount(interval,cell);
        const auto anomaly=classify(interval,cell,std::bit_cast<std::int32_t>(std::uint32_t(index)));
        if(anomaly!=AnomalyKind::None){
            const auto key=anomaly==AnomalyKind::LongInterval?Constants::LongKey:Constants::ShortKey;
            result.structures.push_back({Decoding::StructureKind::TimingAnomaly,std::uint64_t(std::int64_t(std::bit_cast<std::int32_t>(offset))),std::uint64_t(count),Decoding::message(key)});
        }
        offset+=std::uint32_t(count);
    }
    return result;
}
}
