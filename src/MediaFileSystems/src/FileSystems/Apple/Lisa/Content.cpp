#include "Internal.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::Lisa::Internal {
std::map<std::uint16_t,Content> contents(const std::vector<Page>& pages,std::vector<Diagnostic>& warnings,std::stop_token token){
    std::map<std::uint16_t,std::vector<const Page*>> groups;for(const auto& page:pages){checkCancellation(token);if(page.tag&&userFile(page.tag->fileId))groups[page.tag->fileId].push_back(&page);}
    std::map<std::uint16_t,Content> result;
    for(auto& [id,group]:groups){
        checkCancellation(token);std::stable_sort(group.begin(),group.end(),[](const auto* a,const auto* b){return a->tag->pageNumber!=b->tag->pageNumber?a->tag->pageNumber<b->tag->pageNumber:a->block->logicalBlock<b->block->logicalBlock;});
        Content content;content.firstBlock=group.front()->block->logicalBlock;auto expected=group.front()->tag->pageNumber;const auto size=group.front()->block->data.size();std::set<std::int32_t> seen;
        for(const auto* page:group){
            checkCancellation(token);const auto number=page->tag->pageNumber;
            if(!seen.insert(number).second){warnings.push_back({DiagnosticCode::LisaDuplicatePage,{hex(id),std::to_string(number)}});content.valid=false;continue;}
            while(expected<number){checkCancellation(token);warnings.push_back({DiagnosticCode::LisaMissingPage,{hex(id),std::to_string(expected)}});if(size>Constants::MaximumBuffer-content.bytes.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);content.bytes.resize(content.bytes.size()+size,0);++expected;content.valid=false;}
            append(content.bytes,page->block->data);expected=number+1;
        }
        result.emplace(id,std::move(content));
    }
    return result;
}
}
