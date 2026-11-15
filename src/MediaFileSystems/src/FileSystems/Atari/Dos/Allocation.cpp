#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AtariDos {
namespace {
bool contiguous(const std::vector<std::int32_t>& containing,const std::vector<std::int32_t>& candidate){return !candidate.empty()&&candidate.size()<=containing.size()&&std::search(containing.begin(),containing.end(),candidate.begin(),candidate.end())!=containing.end();}
}
std::map<std::int32_t,FileData> analyzeAllocations(const std::vector<DirectoryEntry>& entries,const std::map<std::int32_t,FileData>& files,std::vector<Diagnostic>& warnings,std::stop_token token){
    std::map<std::int32_t,const DirectoryEntry*> byNumber;for(const auto& entry:entries)if(!byNumber.emplace(entry.entryNumber,&entry).second)throw MediaError(DiagnosticCode::ReadOutOfBounds);std::map<std::int32_t,FileData> result;
    for(const auto& entry:entries){
        checkCancellation(token);const auto source=files.find(entry.entryNumber);if(source==files.end())throw MediaError(DiagnosticCode::ReadOutOfBounds);auto file=source->second;
        const auto diagnostic=[&](Diagnostic value){warnings.push_back(value);file.diagnostics.push_back(std::move(value));};
        if(file.chain.continuationSector!=0){diagnostic({DiagnosticCode::AtariDosBounded,{entry.name,std::to_string(entry.declaredSectors),std::to_string(file.chain.continuationSector)}});file.attributes.push_back(std::string(Constants::BoundedSectorViewAttribute));}
        std::vector<std::size_t> mismatches;std::set<std::int32_t> owners;if(file.chain.storedFileNumbers.size()<file.chain.sectors.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);
        for(std::size_t index=0;index<file.chain.sectors.size();++index){const auto owner=file.chain.storedFileNumbers[index];if(owner&&*owner!=entry.entryNumber){mismatches.push_back(index);owners.insert(*owner);}}
        if(!mismatches.empty()){
            const auto ownerEntry=owners.size()==1?byNumber.find(*owners.begin()):byNumber.end();const auto ownerFile=owners.size()==1?files.find(*owners.begin()):files.end();
            if(ownerEntry!=byNumber.end()&&ownerFile!=files.end()&&contiguous(ownerFile->second.chain.sectors,file.chain.sectors)){diagnostic({DiagnosticCode::AtariDosShared,{entry.name,ownerEntry->second->name,std::to_string(file.chain.sectors.front()),std::to_string(file.chain.sectors.size())}});file.attributes.push_back(std::string(Constants::SharedSectorViewAttribute));}
            else{for(const auto index:mismatches)diagnostic({DiagnosticCode::AtariDosOwner,{entry.name,std::to_string(file.chain.sectors[index]),std::to_string(entry.entryNumber),std::to_string(*file.chain.storedFileNumbers[index])}});file.valid=false;}
        }
        std::set<std::string> seen;std::erase_if(file.attributes,[&](const auto& value){return !seen.insert(value).second;});result.emplace(entry.entryNumber,std::move(file));
    }
    return result;
}
}
