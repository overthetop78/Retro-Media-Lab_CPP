#include "ProtectedImage.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Encoding {
Flux::ProtectedTrackImage protectedImage(std::span<const DiskTrack> tracks,std::int32_t resolution,std::stop_token token){
    checkCancellation(token);if(resolution<=0)throw MediaError(DiagnosticCode::ProtectedResolution,{std::to_string(resolution)});
    std::vector<Flux::ProtectedTrack> result;result.reserve(tracks.size());
    for(const auto& track:tracks){
        checkCancellation(token);
        result.emplace_back(track.cylinder,track.head,track.track.bits,std::vector<Flux::TrackTimingSegment>{{0,std::int64_t(track.track.bits.size()),track.cellTicks*double(resolution)}},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::vector<Flux::TrackFluxRevolution>{Flux::TrackFluxRevolution(resolution,track.track.revolution)});
    }
    checkCancellation(token);return {std::move(result),false};
}
}
