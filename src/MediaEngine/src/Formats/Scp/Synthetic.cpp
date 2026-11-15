#include "Writer.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
namespace RetroMediaLab::MediaEngine::Scp {
namespace {
std::uint32_t convertTicks(std::uint64_t ticks,std::uint64_t targetNanoseconds) {
    const auto half=targetNanoseconds/Constants::RoundingDivisor;
    const auto maximum=std::numeric_limits<std::uint64_t>::max();
    if(ticks>(maximum-half)/Encoding::Constants::TickNanoseconds)throw MediaError(DiagnosticCode::ScpOverflow);
    const auto converted=(ticks*Encoding::Constants::TickNanoseconds+half)/targetNanoseconds;
    if(converted>Constants::MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
    return std::uint32_t(converted);
}
}
Revolution syntheticRevolution(const Encoding::EncodedTrack& track,std::uint8_t resolution,std::stop_token token) {
    checkCancellation(token);
    const auto tickNanoseconds=std::uint64_t(Constants::ResolutionStep)*(std::uint32_t(resolution)+1);
    Revolution result;result.origin=RevolutionOrigin::Synthetic;
    result.flux.intervals.reserve(track.revolution.intervals.size());
    std::uint64_t sourceTime=0,previousTargetTime=0;
    for(const auto interval:track.revolution.intervals) {
        checkCancellation(token);
        if(interval>std::numeric_limits<std::uint64_t>::max()-sourceTime)throw MediaError(DiagnosticCode::ScpOverflow);
        sourceTime+=interval;
        const auto targetTime=convertTicks(sourceTime,tickNanoseconds);
        if(targetTime>previousTargetTime) {
            result.flux.intervals.push_back(std::uint32_t(targetTime-previousTargetTime));
            previousTargetTime=targetTime;
        }
    }
    result.flux.indexTimeTicks=convertTicks(track.revolution.indexTimeTicks,tickNanoseconds);
    return result;
}
std::uint8_t trackNumber(std::int32_t cylinder,std::int32_t head) {
    // C# checks the signed multiplication and addition before converting to byte.
    const auto twice=std::int64_t(cylinder)*Constants::HeadCount;
    if(twice<std::numeric_limits<std::int32_t>::min()||twice>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::ScpOverflow);
    const auto combined=twice+head;
    if(combined<0||combined>std::numeric_limits<std::uint8_t>::max())throw MediaError(DiagnosticCode::ScpOverflow);
    return std::uint8_t(combined);
}
}
