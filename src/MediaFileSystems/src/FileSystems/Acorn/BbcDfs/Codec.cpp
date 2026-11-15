#include "Internal.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <locale>
namespace RetroMediaLab::MediaFileSystems::BbcDfs {
using namespace Constants;
CatalogueFields decodeCatalogue(Bytes bytes) {
    if(bytes.size()<FirstEntryOffset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    const auto count=bytes[EntryCountOffset];
    return {std::uint32_t(count/EntryPartSize),std::uint32_t(bytes[TotalSectorsHighOffset]&TotalSectorsHighMask)<<BitsPerByte | bytes[TotalSectorsLowOffset],count%EntryPartSize==0 && count<=MaximumEntryCount*EntryPartSize};
}
EntryFields decodeEntry(Bytes bytes,std::size_t offset) {
    if(offset>bytes.size() || EntryPartSize>bytes.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    const auto packed=bytes[offset+PackedOffset];
    return {std::uint32_t(bytes[offset+LengthOffset]) | std::uint32_t(bytes[offset+LengthOffset+1])<<BitsPerByte | std::uint32_t(packed&LengthHighMask)<<LengthHighShift,
        std::uint32_t(bytes[offset+StartSectorOffset]) | std::uint32_t(packed&StartSectorHighMask)<<BitsPerByte,
        std::uint32_t(bytes[offset]) | std::uint32_t(bytes[offset+1])<<BitsPerByte | std::uint32_t(packed&LoadHighMask)<<LoadHighShift,
        std::uint32_t(bytes[offset+ExecuteOffset]) | std::uint32_t(bytes[offset+ExecuteHighByte])<<BitsPerByte | std::uint32_t(packed&ExecuteHighMask)<<ExecuteHighShift};
}
std::string decodeName(Bytes bytes) {
    std::string result;result.reserve(bytes.size());for(const auto value:bytes)result.push_back(char(value&CharacterMask));
    while(!result.empty() && (result.back()==Null || result.back()==Space))result.pop_back();
    return result;
}
std::string describeAddresses(std::uint32_t load,std::uint32_t execute) {
    std::ostringstream output;output.imbue(std::locale::classic());output<<LoadPrefix<<std::uppercase<<std::hex<<std::setfill(HexPadding)<<std::setw(HexWidth)<<load<<ExecutePrefix<<std::setw(HexWidth)<<execute;return output.str();
}
bool structuredCatalogue(Bytes names,Bytes metadata,std::uint32_t total) {
    if(names.size()!=SectorSize || metadata.size()!=SectorSize)return false;
    const auto header=decodeCatalogue(metadata);
    if(total!=header.totalSectors || !header.countValid)return false;
    const auto count=header.entryCount*EntryPartSize;
    for(std::size_t offset=FirstEntryOffset;offset<count+FirstEntryOffset;offset+=EntryPartSize) {
        for(std::size_t i=0;i<EntryPartSize;++i){const auto value=names[offset+i]&CharacterMask;if(value<PrintableFirst || value>PrintableLast)return false;}
        const auto entry=decodeEntry(metadata,offset);
        if(entry.startSector<CatalogueSectors || entry.startSector+(entry.length+SectorSize-1)/SectorSize>total)return false;
    }
    return true;
}
namespace Internal {
bool same(std::string_view left,std::string_view right) {
    if(left.size()!=right.size())return false;const auto lower=[](char c){return c>=UpperFirst && c<=UpperLast?char(c+CaseDelta):c;};
    return std::equal(left.begin(),left.end(),right.begin(),[&](char a,char b){return lower(a)==lower(b);});
}
bool twoSided(const SectorImage& image){const auto& g=image.geometry();return g.heads==TwoHeads && g.blockSize==SectorSize && g.sectorsPerTrack==SectorsPerTrack;}
bool validIntegrity(const SectorBlock& block){return block.integrityValid!=false && block.headerIntegrityValid!=false && block.dataIntegrityValid!=false;}
}
}
