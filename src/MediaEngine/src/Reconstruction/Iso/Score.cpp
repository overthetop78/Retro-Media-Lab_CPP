#include "Collector.h"
#include "Constants.h"
#include <bit>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
double score(const Decoding::Result& result) noexcept {
    std::uint32_t valid=0,data=0,invalid=0;
    for(const auto& sector:result.sectors){
        if(sector.integrityValid==true)++valid;
        if(sector.data)++data;
        if(sector.integrityValid==false)++invalid;
    }
    const auto weighted=valid*Constants::ValidSectorScoreWeight+data*Constants::DataSectorScoreWeight-invalid*Constants::InvalidSectorScorePenalty;
    return double(std::bit_cast<std::int32_t>(weighted))+result.confidence;
}
}
