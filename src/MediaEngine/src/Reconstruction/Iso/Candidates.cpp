#include "Builder.h"
#include "../../Decoding/Candidates.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
std::vector<Candidate>& Candidates::ensure(const MediaFileSystems::SectorAddress& address){
    const auto key=std::tuple{address.cylinder,address.head,address.number};const auto found=lookup_.find(key);
    if(found!=lookup_.end())return groups_[found->second].candidates;
    groups_.push_back({address,{}});
    try{lookup_.emplace(key,groups_.size()-1);}catch(...){groups_.pop_back();throw;}
    return groups_.back().candidates;
}
void Candidates::add(const MediaFileSystems::SectorAddress& address,Candidate candidate){ensure(address).push_back(std::move(candidate));}
const std::vector<Candidate>* Candidates::find(const MediaFileSystems::SectorAddress& address) const{
    const auto found=lookup_.find({address.cylinder,address.head,address.number});return found==lookup_.end()?nullptr:&groups_[found->second].candidates;
}
const Candidate& best(std::span<const Candidate> candidates,std::stop_token token){
    checkCancellation(token);if(candidates.empty())throw MediaError(DiagnosticCode::IsoEmptyCandidates);
    const Candidate* selected=&candidates.front();
    for(const auto& candidate:candidates){checkCancellation(token);if(Decoding::betterIntegrity(candidate.sector.integrityValid,selected->sector.integrityValid))selected=&candidate;}
    return *selected;
}
std::vector<std::uint8_t> bestData(const Candidates& groups,const MediaFileSystems::SectorAddress& address,std::stop_token token){
    checkCancellation(token);const auto found=groups.find(address);if(!found)return {};
    const Candidate* selected=nullptr;
    for(const auto& candidate:*found){
        checkCancellation(token);if(!candidate.sector.data||candidate.sector.data->empty())continue;
        if(!selected||Decoding::betterIntegrity(candidate.sector.integrityValid,selected->sector.integrityValid))selected=&candidate;
    }
    return selected?*selected->sector.data:std::vector<std::uint8_t>{};
}
}
