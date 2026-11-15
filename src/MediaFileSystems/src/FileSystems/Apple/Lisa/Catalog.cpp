#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Lisa {
namespace Internal {
namespace {
void entry(Bytes data,std::size_t offset,std::size_t length,std::map<std::uint16_t,std::string>& names){
    if(length==0||length>Constants::CatalogNameLength||offset+Constants::CatalogNameOffset+length>data.size())return;
    auto name=Apple::decodeLisaName(data.subspan(offset+Constants::CatalogNameOffset,length));const auto id=Binary::big<std::uint16_t>(data,offset+Constants::CatalogFileIdOffset);
    if(userFile(id)&&!Identifiers::isBlank(name))names.try_emplace(id,std::move(name));
}
}
CatalogResult catalog(const std::vector<Page>& pages,std::uint16_t version,std::stop_token token){
    std::vector<const Page*> selected;for(const auto& page:pages){checkCancellation(token);if(page.tag&&page.tag->fileId==Constants::CatalogFileId)selected.push_back(&page);}
    std::stable_sort(selected.begin(),selected.end(),[](const auto* a,const auto* b){return a->tag->pageNumber<b->tag->pageNumber;});std::vector<std::uint8_t> data;for(const auto* page:selected)append(data,page->block->data);CatalogResult result;
    if(version==std::uint16_t(CatalogVersion::Table)){for(std::size_t offset=0;offset+Constants::TableEntrySize<=data.size();offset+=Constants::TableEntrySize){checkCancellation(token);entry(data,offset,data[offset],result.names);}}
    else{for(std::size_t offset=Constants::TreeEntriesOffset;offset+Constants::TreeEntrySize<=data.size();offset+=Constants::TreeEntrySize){checkCancellation(token);if(data[offset]!=Constants::UnusedCatalogEntryMarker||data[offset+Constants::CatalogNameOffset]<Constants::FirstPrintable)continue;entry(data,offset,Constants::CatalogNameLength,result.names);}}
    return result;
}
}
CatalogResult readCatalog(const SectorImage& image,std::uint16_t version,std::stop_token token){checkCancellation(token);return Internal::catalog(Internal::pages(image,token),version,token);}
}
