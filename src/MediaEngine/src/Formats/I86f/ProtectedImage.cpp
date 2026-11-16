#include "I86f.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::I86f {
Flux::ProtectedTrackImage protectedImage(const Image& image,std::stop_token token) {
    using namespace Constants;checkCancellation(token);if(image.tracks.empty())throw MediaError(DiagnosticCode::I86fNoTracks);
    std::vector<Flux::ProtectedTrack> tracks;tracks.reserve(image.tracks.size());
    for(const auto& track:image.tracks){checkCancellation(token);
        if(track.bits.size()>MaximumFluxTicks/TicksPerCell)throw MediaError(DiagnosticCode::I86fBitCount,{std::to_string(track.logicalIndex),std::to_string(track.bits.size())});
        const auto index=std::uint32_t(track.bits.size()*TicksPerCell);auto revolution=Flux::createRevolution(track.bits,TicksPerCell,index,true,token);
        const auto cylinder=track.logicalIndex%std::int32_t(EntriesPerSide),head=track.logicalIndex/std::int32_t(EntriesPerSide);
        tracks.emplace_back(cylinder,head,track.bits,
            std::vector<Flux::TrackTimingSegment>{{0,std::int64_t(track.bits.size()),double(TicksPerCell)*TickNanoseconds}},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::vector<Flux::TrackFluxRevolution>{Flux::TrackFluxRevolution(TickNanoseconds,std::move(revolution))});
    }return Flux::ProtectedTrackImage(std::move(tracks),false);
}
}
