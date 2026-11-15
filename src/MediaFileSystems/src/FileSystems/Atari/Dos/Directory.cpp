#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AtariDos {
namespace Internal {
bool validEntry(Bytes data,std::size_t offset,std::uint8_t flags,bool validateDeletedName){
    using namespace Constants;if(offset>data.size()||DirectoryEntrySize>data.size()-offset)return false;const bool recorded=(flags&(InUseFlag|OpenForOutputFlag))!=0,deleted=(flags&DeletedFlag)!=0;
    if((flags&~KnownFlags)!=0||(!recorded&&!deleted))return false;if(!validateDeletedName&&(!recorded||deleted))return true;
    bool blank=true;for(std::int32_t character=0;character<NameLength+ExtensionLength;++character){const auto value=data[offset+NameOffset+character];if(value!=0&&value!=NamePadding)blank=false;if(value!=0&&value!=NamePadding&&(value<MinimumNameCharacter||value>MaximumNameCharacter))return false;}
    if(!recorded||deleted)return true;const auto count=Binary::little<std::uint16_t>(data,offset+SectorCountOffset),first=Binary::little<std::uint16_t>(data,offset+FirstSectorOffset);
    return !blank&&(count!=0||first==0||(flags&OpenForOutputFlag)!=0)&&(count==0||first!=0);
}
std::optional<std::vector<DirectoryEntry>> parseEntries(const SectorImage& image,const DirectoryLocation& location,bool realRead,bool requireValid,bool& ended,std::vector<Diagnostic>* warnings,std::stop_token token){
    using namespace Constants;if(location.entriesPerSector<=0)throw MediaError(DiagnosticCode::ReadOutOfBounds);std::vector<DirectoryEntry> result;ended=false;
    for(std::int32_t number=0;number<MaximumEntries;++number){
        checkCancellation(token);const auto sectorNumber=std::int64_t(location.firstSector)+number/location.entriesPerSector;if(sectorNumber>=std::int64_t(location.firstSector)+location.sectorCount)break;if(sectorNumber>std::numeric_limits<std::int32_t>::max()||sectorNumber<std::numeric_limits<std::int32_t>::min())throw MediaError(DiagnosticCode::ReadOutOfBounds);
        std::vector<std::uint8_t> sector;if(!trySector(image,std::int32_t(sectorNumber),sector,token)||(realRead&&sector.size()<MinimumSectorSize)){
            if(!realRead)return {};if(warnings)warnings->push_back({DiagnosticCode::AtariDosDirectoryMissing,{std::to_string(sectorNumber)}});continue;
        }
        const auto slot=number%location.entriesPerSector;const auto offset=std::size_t(slot)*DirectoryEntrySize;if(offset>sector.size()||DirectoryEntrySize>sector.size()-offset){if(!realRead)return {};throw MediaError(DiagnosticCode::ReadOutOfBounds);}
        const auto flags=sector[offset+FlagsOffset];if(flags==0){ended=true;break;}
        if(!validEntry(sector,offset,flags,false)){if(!realRead&&requireValid)return {};if(realRead&&warnings)warnings->push_back({DiagnosticCode::AtariDosDirectoryEntry,{std::to_string(sectorNumber),std::to_string(slot+1)}});continue;}
        if((flags&(InUseFlag|OpenForOutputFlag))==0||(flags&DeletedFlag)!=0)continue;
        auto name=decodeName(Bytes(sector).subspan(offset+NameOffset,NameLength+ExtensionLength));
        if((flags&OpenForOutputFlag)!=0){if(realRead&&warnings)warnings->push_back({DiagnosticCode::AtariDosOpen,{name,std::to_string(sectorNumber),std::to_string(slot+1)}});continue;}
        result.push_back({number&EntryNumberMask,std::int32_t(sectorNumber),slot,std::move(name),DirectoryFlags(flags),Binary::little<std::uint16_t>(sector,offset+SectorCountOffset),Binary::little<std::uint16_t>(sector,offset+FirstSectorOffset)});
    }
    return result;
}
std::map<std::int32_t,FileData> files(const SectorImage& image,const std::vector<DirectoryEntry>& entries,std::vector<Diagnostic>& warnings,std::stop_token token){
    const bool extendedLinks=extended(image,token);std::map<std::int32_t,FileData> result;for(const auto& entry:entries){checkCancellation(token);if(!result.emplace(entry.entryNumber,readFile(image,entry.firstSector,entry.declaredSectors,extendedLinks,warnings,entry.name,token)).second)throw MediaError(DiagnosticCode::ReadOutOfBounds);}return result;
}
bool analyzeDirectory(const SectorImage& image,const DirectoryLocation& location,bool relocated,std::int32_t& recorded,std::int32_t& readable,std::stop_token token){
    recorded=readable=0;bool ended=false;const auto entries=parseEntries(image,location,false,relocated,ended,nullptr,token);if(!entries)return false;recorded=std::int32_t(entries->size());if(!ended&&recorded<Constants::MaximumEntries)return false;std::vector<Diagnostic> warnings;const auto decoded=files(image,*entries,warnings,token);const auto analyzed=analyzeAllocations(*entries,decoded,warnings,token);for(const auto&[number,file]:analyzed)if(file.valid)++readable;
    return relocated?recorded>=Constants::MinimumRelocatedEntries&&readable>=Constants::MinimumRelocatedEntries&&std::int64_t(readable)*Constants::RelocatedMajorityMultiplier>=recorded:recorded>=1&&readable>=1;
}
}
std::optional<DirectoryLocation> locateDirectory(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto canonical=Internal::canonical();std::int32_t recorded=0,readable=0;if(Internal::analyzeDirectory(image,canonical,false,recorded,readable,token))return canonical;
    std::optional<DirectoryLocation> best;std::int32_t bestReadable=0,bestRecorded=0;
    for(std::uint64_t first=1;first<=image.blockCount();++first){
        checkCancellation(token);if(first>=Constants::VtocSector&&first<=Constants::LastDirectorySector)continue;if(first>std::uint64_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::ReadOutOfBounds);
        std::vector<std::uint8_t> data;if(!trySector(image,std::int32_t(first),data,token)||data.size()<Constants::MinimumSectorSize)continue;const auto perSector=data.size()/Constants::DirectoryEntrySize;if(perSector<Constants::DirectoryEntriesPerSector||perSector>std::size_t(std::numeric_limits<std::int32_t>::max()))continue;const auto count=(Constants::MaximumEntries+perSector-1)/perSector;DirectoryLocation candidate{std::int32_t(first),std::int32_t(count),std::int32_t(perSector)};
        if(!Internal::analyzeDirectory(image,candidate,true,recorded,readable,token))continue;if(readable<bestReadable||(readable==bestReadable&&recorded<=bestRecorded))continue;best=candidate;bestReadable=readable;bestRecorded=recorded;
    }
    return best;
}
bool containsRecordedEntry(const SectorImage& image,std::stop_token token){checkCancellation(token);bool ended=false;const auto entries=Internal::parseEntries(image,Internal::canonical(),false,false,ended,nullptr,token);return entries&&!entries->empty();}
bool containsReadableEntry(const SectorImage& image,std::stop_token token){checkCancellation(token);std::int32_t recorded=0,readable=0;return Internal::analyzeDirectory(image,Internal::canonical(),false,recorded,readable,token)&&readable!=0;}
std::vector<FileSystemEntry> readDirectory(const SectorImage& image,std::vector<Diagnostic>& warnings,std::stop_token token){checkCancellation(token);const auto location=locateDirectory(image,token);return location?readDirectory(image,*location,warnings,token):std::vector<FileSystemEntry>{};}
std::vector<FileSystemEntry> readDirectory(const SectorImage& image,const DirectoryLocation& location,std::vector<Diagnostic>& warnings,std::stop_token token){
    checkCancellation(token);bool ended=false;const auto entries=Internal::parseEntries(image,location,true,false,ended,&warnings,token);const auto files=Internal::files(image,*entries,warnings,token);auto analyzed=analyzeAllocations(*entries,files,warnings,token);std::vector<FileSystemEntry> result;
    for(const auto& entry:*entries){checkCancellation(token);const auto found=analyzed.find(entry.entryNumber);if(found==analyzed.end())throw MediaError(DiagnosticCode::ReadOutOfBounds);auto& file=found->second;FileSystemEntry value;value.name=entry.name;value.kind=EntryKind::File;value.size=file.content.size();value.rawAttributes=std::uint8_t(entry.flags);value.storageReference=entry.firstSector;value.metadataValid=true;value.content=std::move(file.content);value.attributes=std::move(file.attributes);value.dataValid=file.valid;value.diagnostics=std::move(file.diagnostics);result.push_back(std::move(value));}
    std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){return Identifiers::utf8(a.name).compare(Identifiers::utf8(b.name),Qt::CaseInsensitive)<0;});return result;
}
}
