#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AtariDos {
FileData readFile(const SectorImage& image,std::int32_t first,std::int32_t expected,bool extendedLinks,std::vector<Diagnostic>& warnings,std::string_view name,std::stop_token token){
    FileData result;auto current=first;std::set<std::int32_t> visited;bool valid=true;
    while(std::int64_t(result.chain.sectors.size())<expected){
        checkCancellation(token);if(current==0){warnings.push_back({DiagnosticCode::AtariDosPremature,{std::string(name),std::to_string(expected),std::to_string(result.chain.sectors.size())}});valid=false;break;}
        if(!visited.insert(current).second){warnings.push_back({DiagnosticCode::AtariDosChainCycle,{std::string(name),std::to_string(current)}});valid=false;break;}
        std::vector<std::uint8_t> sector;if(!trySector(image,current,sector,token)){warnings.push_back({DiagnosticCode::AtariDosDataMissing,{std::string(name),std::to_string(current)}});valid=false;break;}
        if(sector.size()<Constants::LinkByteCount){warnings.push_back({DiagnosticCode::AtariDosTruncated,{std::string(name),std::to_string(current)}});valid=false;break;}
        const auto link=sector.size()-Constants::LinkByteCount;const auto high=extendedLinks?sector[link]:sector[link]&Constants::NextSectorHighMask;const auto next=(std::int32_t(high)<<Constants::BitsPerByte)|sector[link+Constants::LinkLowOffset];auto used=std::size_t(sector[link+Constants::LinkUsedOffset]);
        if(used>link){warnings.push_back({DiagnosticCode::AtariDosUsedLength,{std::string(name),std::to_string(current),std::to_string(used),std::to_string(link)}});used=link;valid=false;}
        result.chain.sectors.push_back(current);result.chain.storedFileNumbers.push_back(extendedLinks?std::optional<std::int32_t>{}:std::optional<std::int32_t>{sector[link]>>Constants::FileOwnerShift});
        if(used>Constants::MaximumBuffer-result.content.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);result.content.insert(result.content.end(),sector.begin(),sector.begin()+used);current=next;
    }
    checkCancellation(token);if(expected==0&&current!=0){warnings.push_back({DiagnosticCode::AtariDosEmptyData,{std::string(name),std::to_string(current)}});valid=false;}
    result.chain.continuationSector=current;result.chain.complete=std::int64_t(result.chain.sectors.size())==expected;result.valid=valid&&result.chain.complete;return result;
}
}
