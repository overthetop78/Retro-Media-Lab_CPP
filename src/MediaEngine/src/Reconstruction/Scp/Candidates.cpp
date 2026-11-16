#include "Candidates.h"
#include "NamesConstants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
std::string_view Candidate::displayName()const {
    for(const auto& [identifier,name]:NamesConstants::Exact)if(id==identifier)return name;
    const auto text=std::string_view(id);
    for(const auto& [pattern,name]:NamesConstants::Patterns){
        if(pattern.size()>text.size())continue;
        for(std::size_t offset=0;offset<=text.size()-pattern.size();++offset)
            if(Primitives::asciiIdentifierEqualsIgnoreCase(text.substr(offset,pattern.size()),pattern))return name;
    }
    const auto index=static_cast<std::size_t>(family);
    return index<NamesConstants::Families.size()?NamesConstants::Families[index]:std::string_view{};
}
Candidates::Candidates(std::vector<Selection> selections,std::vector<Candidate> defaults,std::map<Family,std::vector<Candidate>> families,std::vector<Family> order,Candidate fallback)
    :selections_(std::move(selections)),defaults_(std::move(defaults)),families_(std::move(families)),order_(std::move(order)),fallback_(std::move(fallback)) {
    const auto validate=[](const Candidate& candidate){if(!candidate.readPath)throw MediaError(DiagnosticCode::ReaderRegistryNull);};
    validate(fallback_);
    for(const auto& selection:selections_){if(!selection.matches)throw MediaError(DiagnosticCode::ReaderRegistryNull);validate(selection.candidate);}
    for(const auto& candidate:defaults_)validate(candidate);
    for(const auto& [family,candidates]:families_){(void)family;for(const auto& candidate:candidates)validate(candidate);}
}
const Candidate* Candidates::selected(const std::optional<std::string>& format)const {
    if(!format)return nullptr;
    for(const auto& selection:selections_)if(selection.matches(*format))return &selection.candidate;
    return &fallback_;
}
std::span<const Candidate> Candidates::defaults()const noexcept{return defaults_;}
std::vector<Candidate> Candidates::automatic(const std::set<Family>& families)const {
    std::vector<Candidate> result;
    for(const auto family:order_){
        if(!families.empty()&&!families.contains(family))continue;
        if(const auto entry=families_.find(family);entry!=families_.end())result.insert(result.end(),entry->second.begin(),entry->second.end());
    }
    return result;
}
}
