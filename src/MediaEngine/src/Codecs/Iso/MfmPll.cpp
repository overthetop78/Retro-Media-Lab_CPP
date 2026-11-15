#include "MfmRead.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <bit>
#include <limits>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::IsoMfm::Reading {
std::int32_t score(const Decoding::Result& result) noexcept{
    const auto valid=std::uint32_t(std::count_if(result.sectors.begin(),result.sectors.end(),[](const auto& sector){return sector.integrityValid==true;}));
    const auto data=std::uint32_t(std::count_if(result.sectors.begin(),result.sectors.end(),[](const auto& sector){return sector.data.has_value();}));
    return std::bit_cast<std::int32_t>(valid*Constants::ValidScoreWeight+data*Constants::DataScoreWeight+std::uint32_t(result.sectors.size())*Constants::SectorScoreWeight);
}
Decoding::Result select(const Flux::Revolution& revolution,double centre,const std::function<Decoding::Result(const Flux::Bitstream&)>& decode,std::stop_token token){
    std::optional<Decoding::Result> best;auto bestScore=std::numeric_limits<std::int32_t>::min();
    for(auto factor:Constants::PllFactors){
        checkCancellation(token);auto candidate=decode(Flux::decodePll(revolution.intervals,centre*factor,token));
        if(factor==1&&!candidate.sectors.empty()&&std::all_of(candidate.sectors.begin(),candidate.sectors.end(),[](const auto& sector){return sector.data.has_value()&&sector.integrityValid==true;}))return candidate;
        const auto candidateScore=score(candidate);if(candidateScore<=bestScore)continue;bestScore=candidateScore;best=std::move(candidate);
    }
    return std::move(*best);
}
}
namespace RetroMediaLab::MediaEngine::IsoMfm {
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){const auto centre=Flux::estimateCell(revolution.intervals,Flux::TimingMode::NonFm,token);return Reading::select(revolution,centre,[&](const auto& stream){return decodeCore(stream,token);},token);}
}
