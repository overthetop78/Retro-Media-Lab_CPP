#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Cpm {
std::span<const std::string_view> catalogFormatIds() noexcept{return Constants::GenericFormats;}
std::span<const std::string_view> amstradCatalogFormatIds() noexcept{return Constants::AmstradFormats;}
std::optional<AmstradDiskSpecification> parseAmstradSpecification(Bytes bytes){
    using namespace Constants;if(bytes.size()<SpecificationMinimumLength)return {};
    const auto tracks=bytes[TracksOffset],sectors=bytes[SectorsPerTrackOffset],reserved=bytes[ReservedTracksOffset],blocks=bytes[DirectoryBlocksOffset];
    const auto sector=MinimumSectorSize<<(bytes[SectorSizeCodeOffset]&SizeCodeMask),allocation=MinimumSectorSize<<(bytes[AllocationSizeCodeOffset]&SizeCodeMask);
    if(tracks==0||tracks>MaximumTracks||sectors==0||sectors>MaximumSectorsPerTrack||sector<MinimumSectorSize||sector>MaximumSectorSize||reserved>MaximumReservedTracks||allocation<MinimumAllocationSize||allocation>MaximumAllocationSize||blocks==0||blocks>MaximumDirectoryBlocks)return {};
    return AmstradDiskSpecification{tracks,sectors,sector,reserved,allocation,blocks};
}
namespace Internal {
Layout::Layout(std::int32_t directory,std::int32_t origin,std::int32_t entries,std::int32_t size,std::int32_t blocks,bool wideAllocations):directoryOffset(directory),allocationOrigin(origin),directoryEntries(entries),allocationSize(size),directoryBlocks(blocks),wide(wideAllocations){
    if(directory<0||origin<0||entries<1||size<1||blocks<1)throw MediaError(DiagnosticCode::CpmInvalidLayout,{std::to_string(directory),std::to_string(origin),std::to_string(entries),std::to_string(size),std::to_string(blocks)});
}
bool equal(std::string_view a,std::string_view b){return Identifiers::utf8(a).compare(Identifiers::utf8(b),Qt::CaseInsensitive)==0;}
bool lessName(std::string_view a,std::string_view b){return Identifiers::utf8(a).compare(Identifiers::utf8(b),Qt::CaseInsensitive)<0;}
std::optional<Layout> genericLayout(std::string_view format){
    for(std::size_t index=0;index<Constants::GenericFormats.size();++index)if(equal(format,Constants::GenericFormats[index])){const auto& [directory,origin,entries,size,blocks,wide]=Constants::GenericLayouts[index];return Layout(directory,origin,entries,size,blocks,wide!=0);}return {};
}
Layout epsonLayout(std::string_view format,const LogicalImage& image,const Layout& configured,std::stop_token token){
    if(!Identifiers::utf8(format).startsWith(Identifiers::utf8(Formats::EpsonQx10Prefix),Qt::CaseInsensitive))return configured;
    auto best=configured;auto bestScore=score(image,configured,true,token);const auto limit=std::min(std::int64_t(image.bytes.size())-std::int64_t(configured.directoryEntries)*Constants::DirectoryEntrySize,std::int64_t(Constants::MaximumEpsonDirectorySearch));
    for(std::int64_t offset=0;offset<=limit;offset+=Constants::DirectoryEntrySize){checkCancellation(token);auto candidate=configured;candidate.directoryOffset=std::int32_t(offset);const auto value=score(image,candidate,true,token);if(value<=bestScore)continue;best=candidate;bestScore=value;}return best;
}
std::optional<Layout> amstradLayout(const SectorImage& image,const LogicalImage& logical,std::stop_token token){
    using namespace Constants;
    if(equal(image.geometry().formatId,Formats::AmstradCpc)||equal(image.geometry().formatId,Formats::CpcEmuDsk)){
        std::shared_ptr<const SectorBlock> first;image.visitAvailableBlocks([&](auto number){checkCancellation(token);auto block=image.block(number,token);if(block&&(!first||block->logicalBlock<first->logicalBlock))first=std::move(block);return true;},token);if(!first)return {};
        std::optional<Layout> preferred;const auto sector=first->address.number;
        if(sector>=CpcSystemFirst&&sector<=CpcSystemLast)preferred=Layout(CpcSystemReserved*CpcSectorsPerTrack*CpcSectorSize,CpcSystemReserved*CpcSectorsPerTrack*CpcSectorSize,CpcDirectoryEntries,CpcAllocationSize,CpcDirectoryBlocks,false);
        else if(sector>=CpcDataFirst&&sector<=CpcDataLast)preferred=Layout(0,0,CpcDirectoryEntries,CpcAllocationSize,CpcDirectoryBlocks,false);
        if(preferred&&looksLike(logical,*preferred,false,false,token))return preferred;
        return findDirectory(logical,Layout(0,0,CpcDirectoryEntries,CpcAllocationSize,CpcDirectoryBlocks,false),CpcSectorSize,false,false,token);
    }
    const auto specification=parseAmstradSpecification(logical.bytes);if(!specification)return {};
    const auto& value=*specification;const auto origin=std::int32_t(value.reservedTracks)*value.sectorsPerTrack*value.sectorSize;const auto count=std::max(std::int64_t(0),(std::int64_t(logical.bytes.size())-origin)/value.allocationSize);
    return Layout(origin,origin,value.directoryBlocks*value.allocationSize/DirectoryEntrySize,value.allocationSize,value.directoryBlocks,count>std::numeric_limits<std::uint8_t>::max());
}
}
bool looksLikeCpcSystemDirectory(const SectorImage& image,std::stop_token token){using namespace Constants;checkCancellation(token);const auto logical=Internal::flatten(image,token);return Internal::findDirectory(logical,Internal::Layout(CpcSystemReserved*CpcSectorsPerTrack*CpcSectorSize,CpcSystemReserved*CpcSectorsPerTrack*CpcSectorSize,CpcDirectoryEntries,CpcAllocationSize,CpcDirectoryBlocks,false),CpcSectorSize,false,false,token).has_value();}
}
