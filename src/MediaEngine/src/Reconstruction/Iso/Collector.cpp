#include "Collector.h"
#include "../../Formats/Scp/Scp.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <cmath>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
CandidateSet collectTrack(const Flux::ProtectedTrack& track,std::span<const std::string> ids,const DecodeFlux& decode,const std::function<void(std::int32_t)>& progress,std::stop_token token){
    CandidateSet candidates;
    const auto windows=Scp::decodeWindows(track,token);
    for(const auto& window:windows){
        checkCancellation(token);
        if(ids.empty())throw MediaError(DiagnosticCode::IsoNoDecoders);
        if(!decode)throw MediaError(DiagnosticCode::IsoDecoderMissing);
        const auto revolution=std::make_shared<const Flux::Revolution>(window.flux);
        std::optional<Decoding::Result> selected;double bestScore=0;
        for(const auto& id:ids){
            auto result=decode(id,revolution);const auto current=score(result);
            // .NET descending double order puts NaN after every real value; ties stay first.
            if(!selected||current>bestScore||(std::isnan(bestScore)&&!std::isnan(current))){selected=std::move(result);bestScore=current;}
        }
        for(const auto& sector:selected->sectors){
            if(!sector.data||sector.number<0)continue;
            candidates.physical.add({track.cylinder(),track.head(),sector.number},{sector,window.revolution,{}});
            if(sector.cylinder!=track.cylinder()||sector.head!=track.head())continue;
            candidates.addressed.add({sector.cylinder,sector.head,sector.number},{sector,window.revolution,{}});
        }
        if(progress)progress(window.revolution);
    }
    return candidates;
}
}
