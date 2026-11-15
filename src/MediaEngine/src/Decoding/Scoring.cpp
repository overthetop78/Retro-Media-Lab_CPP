#include "Scoring.h"
#include "ScoringConstants.h"
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine::Decoding {
namespace {
int compareDouble(double left,double right) noexcept {
    if(std::isnan(left))return std::isnan(right)?0:-1;
    if(std::isnan(right))return 1;
    return left>right?1:left<right?-1:0;
}
}
ExplicitScore explicitScore(const Result& result) noexcept {
    ExplicitScore score;
    for(const auto& sector:result.sectors){
        if(sector.integrityValid==true)++score.validSectors;
        if(sector.integrityValid==false)--score.invalidPenalty;
        if(sector.data)++score.sectorsWithData;
    }
    score.confidence=result.confidence;score.structures=std::int32_t(result.structures.size());
    return score;
}
double automaticScore(const Result& result) noexcept {
    using namespace ScoringConstants;
    const auto score=explicitScore(result);
    const auto valid=score.validSectors,invalid=-score.invalidPenalty;
    if(valid>0)return ValidSectorBaseScore+valid/double(std::max(1,valid+invalid))*ValidSectorRatioWeight+result.confidence*ValidSectorConfidenceWeight;
    if(!result.sectors.empty()&&invalid==0)return UnverifiedSectorBaseScore+result.confidence;
    if(invalid>0)return result.confidence*InvalidSectorConfidenceWeight;
    if(result.decoderId==RawDecoderId)return RawFluxBaseScore+result.confidence;
    if(!result.structures.empty())return StructuredFluxBaseScore+result.confidence;
    return result.confidence;
}
bool betterAutomatic(const Result& left,const Result& right) noexcept {
    const auto primary=compareDouble(automaticScore(left),automaticScore(right));if(primary)return primary>0;
    const auto confidence=compareDouble(left.confidence,right.confidence);if(confidence)return confidence>0;
    return left.structures.size()>right.structures.size();
}
bool betterExplicit(const Result& left,const Result& right) noexcept {
    const auto a=explicitScore(left),b=explicitScore(right);
    if(a.validSectors!=b.validSectors)return a.validSectors>b.validSectors;
    if(a.invalidPenalty!=b.invalidPenalty)return a.invalidPenalty>b.invalidPenalty;
    if(a.sectorsWithData!=b.sectorsWithData)return a.sectorsWithData>b.sectorsWithData;
    const auto confidence=compareDouble(a.confidence,b.confidence);if(confidence)return confidence>0;
    return a.structures>b.structures;
}
}
