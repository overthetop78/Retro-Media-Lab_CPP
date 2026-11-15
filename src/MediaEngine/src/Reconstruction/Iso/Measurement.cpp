#include "Builder.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <bit>
#include <set>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
Measurement measure(const Candidates& groups,std::stop_token token){
    checkCancellation(token);if(groups.empty())throw MediaError(DiagnosticCode::IsoNoAddressedCandidates);
    std::vector<std::pair<std::int32_t,std::size_t>> sizes;
    std::map<std::int32_t,std::size_t> sizeIndices;
    std::map<std::pair<std::int64_t,std::int64_t>,std::set<std::int64_t>> tracks;
    std::set<std::int64_t> order;auto maxCylinder=groups.entries().front().address.cylinder,maxHead=groups.entries().front().address.head;
    for(const auto& group:groups.entries()){
        checkCancellation(token);maxCylinder=std::max(maxCylinder,group.address.cylinder);maxHead=std::max(maxHead,group.address.head);tracks[{group.address.cylinder,group.address.head}].insert(group.address.number);order.insert(group.address.number);
        for(const auto& candidate:group.candidates){
            checkCancellation(token);if(!candidate.sector.data)throw MediaError(DiagnosticCode::IsoCandidateData);
            const auto size=std::int32_t(candidate.sector.data->size());const auto [it,inserted]=sizeIndices.emplace(size,sizes.size());if(inserted)sizes.push_back({size,0});++sizes[it->second].second;
        }
    }
    if(sizes.empty())throw MediaError(DiagnosticCode::IsoEmptyCandidates);
    auto selected=sizes.front();for(const auto& entry:sizes)if(entry.second>selected.second)selected=entry;
    std::map<std::size_t,std::size_t> frequencies;for(const auto& [track,numbers]:tracks)++frequencies[numbers.size()];
    std::size_t sectors=0,frequency=0;for(const auto& [count,occurrences]:frequencies)if(occurrences>frequency||(occurrences==frequency&&count>sectors)){sectors=count;frequency=occurrences;}
    Measurement result;result.sectorSize=selected.first;result.cylinders=std::bit_cast<std::int32_t>(std::uint32_t(maxCylinder)+1);result.heads=std::bit_cast<std::int32_t>(std::uint32_t(maxHead)+1);result.sectorsPerTrack=std::int32_t(sectors);result.sectorOrder.assign(order.begin(),order.end());result.zeroBased=!order.empty()&&*order.begin()==0;return result;
}
}
