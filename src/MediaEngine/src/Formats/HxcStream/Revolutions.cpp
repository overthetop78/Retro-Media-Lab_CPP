#include "Internal.h"
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
std::uint64_t add(std::uint64_t a,std::uint64_t b){if(b>Constants::MaximumTime-a)throw MediaError(DiagnosticCode::HxcDuration);return a+b;}
std::uint32_t ticks(std::uint64_t value){if(value>Constants::MaximumRevolutionTicks)throw MediaError(DiagnosticCode::HxcDuration);return std::uint32_t(value);}
}
std::vector<Flux::TrackFluxRevolution> createRevolutions(std::span<const std::uint32_t> flux,std::span<const std::uint16_t> io,std::int32_t resolution,std::stop_token token) {
    using namespace Constants;std::vector<std::uint64_t> indexes;bool previous=false;
    for(std::size_t i=0;i<io.size();++i){checkCancellation(token);const bool current=(io[i]&IndexFlag)!=0;if(current&&!previous)indexes.push_back(std::uint64_t(i)*IoSampleTicks);previous=current;}
    std::vector<Flux::TrackFluxRevolution> revolutions;
    if(indexes.size()>=MinimumIndexEdges){std::vector<std::uint64_t> absolute;absolute.reserve(flux.size());std::uint64_t time{};for(const auto value:flux){checkCancellation(token);time=add(time,value);absolute.push_back(time);}
        std::size_t pulse{};
        for(std::size_t index=1;index<indexes.size();++index){checkCancellation(token);const auto start=indexes[index-1],end=indexes[index];while(pulse<absolute.size()&&absolute[pulse]<=start)++pulse;
            std::vector<std::uint32_t> intervals;auto prior=start;
            while(pulse<absolute.size()&&absolute[pulse]<=end){intervals.push_back(ticks(absolute[pulse]-prior));prior=absolute[pulse++];}
            revolutions.emplace_back(resolution,Flux::Revolution{ticks(end-start),std::move(intervals)});
        }
    }else if(!flux.empty()){
        std::uint64_t duration{};for(const auto value:flux){checkCancellation(token);duration=add(duration,value);}
        revolutions.emplace_back(resolution,Flux::Revolution{ticks(duration),{flux.begin(),flux.end()}});
    }return revolutions;
}
}
