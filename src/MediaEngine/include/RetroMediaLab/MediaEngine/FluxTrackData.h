#pragma once
#include "Export.h"
#include <cstdint>
#include <span>
#include <vector>
namespace RetroMediaLab::MediaEngine {
class FluxRevolutionData final {
public:
    RETROMEDIALAB_MEDIAENGINE_API FluxRevolutionData(std::uint32_t indexTimeNanoseconds,std::vector<std::uint32_t> intervalsNanoseconds);
    std::uint32_t indexTimeNanoseconds()const noexcept{return period_;}
    std::span<const std::uint32_t> intervalsNanoseconds()const noexcept{return intervals_;}
private:
    std::uint32_t period_;std::vector<std::uint32_t> intervals_;
};
class FluxTrackData final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit FluxTrackData(std::vector<FluxRevolutionData>);
    std::span<const FluxRevolutionData> revolutions()const noexcept{return revolutions_;}
private:
    std::vector<FluxRevolutionData> revolutions_;
};
RETROMEDIALAB_MEDIAENGINE_API FluxRevolutionData replaceFluxWindow(const FluxRevolutionData&,std::uint32_t startNanoseconds,std::uint32_t durationNanoseconds,std::span<const std::uint32_t> transitionOffsetsNanoseconds,bool includeEndingInstant=false);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> serializeFluxTrack(const FluxTrackData&);
RETROMEDIALAB_MEDIAENGINE_API FluxTrackData deserializeFluxTrack(std::span<const std::uint8_t>);
}
