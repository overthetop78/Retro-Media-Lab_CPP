#include "Collector.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <atomic>
#include <exception>
#include <limits>
#include <mutex>
#include <thread>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
CandidateSet collect(const Flux::ProtectedTrackImage& image,std::span<const std::string> ids,const DecodeFlux& decode,const CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);
    const auto& tracks=image.tracks();std::int32_t totalRevolutions=0;
    for(const auto& track:tracks){
        if(track.revolutions().size()>std::size_t(std::numeric_limits<std::int32_t>::max()-totalRevolutions))throw MediaError(DiagnosticCode::ScpOverflow);
        totalRevolutions+=std::int32_t(track.revolutions().size());
    }
    std::vector<CandidateSet> results(tracks.size());std::atomic_size_t next{0};std::atomic_int32_t completedTracks{0},completedRevolutions{0};
    std::stop_source failureStop;std::stop_callback forwardCancellation(token,[&]{failureStop.request_stop();});
    std::mutex failureGate;std::exception_ptr failure;
    const auto run=[&]{
        try{
            for(;;){
                checkCancellation(failureStop.get_token());const auto index=next.fetch_add(1);if(index>=tracks.size())return;
                const auto& track=tracks[index];
                results[index]=collectTrack(track,ids,decode,[&](auto revolution){
                    const auto count=completedRevolutions.fetch_add(1)+1;
                    if(progress)progress({CollectionProgressKind::RevolutionDecoded,count,totalRevolutions,track.cylinder(),track.head(),revolution,std::int32_t(track.revolutions().size())});
                },failureStop.get_token());
                const auto count=completedTracks.fetch_add(1)+1;
                if(progress)progress({CollectionProgressKind::TrackDecoded,count,std::int32_t(tracks.size()),track.cylinder(),track.head(),0,std::int32_t(track.revolutions().size())});
            }
        }catch(...){
            {std::lock_guard lock(failureGate);if(!failure)failure=std::current_exception();}
            failureStop.request_stop();
        }
    };
    const auto parallelism=std::min(tracks.size(),std::size_t(std::max(1u,std::thread::hardware_concurrency())));
    {
        std::vector<std::jthread> workers;workers.reserve(parallelism);
        for(std::size_t index=0;index<parallelism;++index)workers.emplace_back(run);
    }
    if(failure)std::rethrow_exception(failure);
    CandidateSet merged;
    const auto append=[](Candidates& destination,const Candidates& source){
        for(const auto& group:source.entries())for(const auto& candidate:group.candidates)destination.add(group.address,candidate);
    };
    for(const auto& result:results){append(merged.addressed,result.addressed);append(merged.physical,result.physical);}
    return merged;
}
}
