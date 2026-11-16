#include <RetroMediaLab/MediaEngine/DiskClassification.h>
#include "../../Visualization/ClassificationConstants.h"
#include "../../Primitives/Identifiers.h"
#include <QCollator>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
namespace C=Visualization::ClassificationConstants;
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
bool matches(std::string_view value,std::optional<std::string_view> requested){return requested&&same(value,*requested);}
}
DiskClassificationCatalog::DiskClassificationCatalog(std::vector<DiskFormat> formats){
 for(auto& format:formats){if(format.id==C::RawScp)continue;if(std::none_of(machines_.begin(),machines_.end(),[&](const auto& machine){return same(machine,format.family);}))machines_.push_back(format.family);formats_.push_back(std::move(format));}
 QCollator collator;collator.setCaseSensitivity(Qt::CaseInsensitive);std::stable_sort(machines_.begin(),machines_.end(),[&](const auto& a,const auto& b){return collator.compare(Primitives::utf8Identifier(a),Primitives::utf8Identifier(b))<0;});
 protections_.push_back({std::string(Formats::AppleIIRwts18),std::string(C::AppleII),{std::string(Formats::AppleIIAppleDos140)},std::string(C::Rwts18Name)});
}
DiskClassificationCatalog DiskClassificationCatalog::createDefault(){const auto formats=imageFormats();return DiskClassificationCatalog({formats.begin(),formats.end()});}
const DiskFormat* DiskClassificationCatalog::resolveFormat(std::optional<std::string_view> id)const{
 if(!id||Primitives::isBlankIdentifier(*id))return nullptr;const auto find=[&](std::string_view requested)->const DiskFormat*{const auto found=std::find_if(formats_.begin(),formats_.end(),[&](const auto& format){return same(format.id,requested);});return found==formats_.end()?nullptr:&*found;};
 if(const auto exact=find(*id))return exact;for(const auto& [alias,target]:C::FormatAliases)if(same(alias,*id))return find(target);return nullptr;
}
std::vector<DiskFormat> DiskClassificationCatalog::formatsFor(std::optional<std::string_view> machine)const{std::vector<DiskFormat> result;for(const auto& format:formats_)if(matches(format.family,machine))result.push_back(format);return result;}
std::vector<DiskProtection> DiskClassificationCatalog::protectionsFor(std::optional<std::string_view> machine,std::optional<std::string_view> format)const{std::vector<DiskProtection> result;for(const auto& protection:protections_)if(matches(protection.machine,machine)&&format&&std::any_of(protection.formatIds.begin(),protection.formatIds.end(),[&](const auto& id){return same(id,*format);}))result.push_back(protection);return result;}
}
