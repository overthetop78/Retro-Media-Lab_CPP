#include "Flux.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Flux {
double estimateCell(Intervals intervals,TimingMode mode,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(intervals.empty())return FallbackCell;
 std::vector<std::uint32_t> sorted;const auto collect=[&](std::size_t first){for(auto index=first;index<intervals.size();++index){checkCancellation(token);if(intervals[index])sorted.push_back(intervals[index]);}};
 collect(mode==TimingMode::Fm?0:1);if(sorted.empty())collect(0);if(sorted.empty())return FallbackCell;
 std::sort(sorted.begin(),sorted.end());checkCancellation(token);
 if(mode==TimingMode::NonFm){const auto count=std::max(std::size_t(1),sorted.size()/LowerClusterDivisor);return std::max(MinimumCell,sorted[count/MedianDivisor]/RobustDivisor);}
 return std::max(MinimumCell,double(sorted[std::min(sorted.size()/PercentileDivisor,sorted.size()-1)]));
}
}
