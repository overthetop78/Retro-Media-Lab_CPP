#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::BbcDfs {
using namespace Constants;
namespace Internal {
bool canReadSingle(const CatalogueView& image,std::stop_token token) {
    checkCancellation(token);
    if(image.geometry().blockSize!=SectorSize)return false;
    const auto names=image.block(NamesSector,token),metadata=image.block(MetadataSector,token);
    if(!names || !metadata || names->data.size()!=SectorSize || metadata->data.size()!=SectorSize)return false;
    const auto header=decodeCatalogue(metadata->data);const auto total=header.totalSectors;
    const auto known=std::any_of(Supported.begin(),Supported.end(),[&](auto id){return same(id,image.geometry().formatId);});
    if(!known && (image.geometry().sectorsPerTrack!=SectorsPerTrack || (total!=SmallSectorCount && total!=LargeSectorCount) || !structuredCatalogue(names->data,metadata->data,total)))return false;
    return header.countValid && total<=image.blockCount();
}
FileSystemVolume readSingle(const CatalogueView& image,std::stop_token token) {
    if(!canReadSingle(image,token)) {
        const auto metadata=image.block(MetadataSector,token);
        const auto observed=metadata && metadata->data.size()>EntryCountOffset?std::int64_t(metadata->data[EntryCountOffset]/EntryPartSize):-1;
        throw MediaError(DiagnosticCode::BbcInvalidCatalog,{std::to_string(observed),std::to_string(image.blockCount())});
    }
    const auto namesBlock=image.block(NamesSector,token),metadataBlock=image.block(MetadataSector,token);
    const Bytes names=namesBlock->data,metadata=metadataBlock->data;
    const auto header=decodeCatalogue(metadata);auto total=std::uint64_t(header.totalSectors);
    if(!total && same(image.geometry().formatId,AcornAtomDos))total=image.blockCount();
    FileSystemVolume result;result.fileSystemId=FileSystemId;applyFileSystemDisplayName(result,{});
    const auto title=decodeName(names.first(TitleFirstLength))+decodeName(metadata.first(TitleSecondLength));
    result.name=QString::fromUtf8(title).trimmed().toUtf8().toStdString();
    const bool catalogueValid=validIntegrity(*namesBlock) && validIntegrity(*metadataBlock);
    if(!validIntegrity(*namesBlock))result.warnings.push_back({DiagnosticCode::BbcIntegrityInvalid,{std::to_string(NamesSector)}});
    if(!validIntegrity(*metadataBlock))result.warnings.push_back({DiagnosticCode::BbcIntegrityInvalid,{std::to_string(MetadataSector)}});
    std::uint64_t used=CatalogueSectors;
    const auto count=header.entryCount;
    for(std::uint32_t index=0;index<count;++index) {
        checkCancellation(token);
        const auto offset=FirstEntryOffset+index*EntryPartSize;
        const auto fields=decodeEntry(metadata,offset);
        const auto leaf=decodeName(names.subspan(offset,LeafNameLength));
        const auto directoryByte=names[offset+DirectoryOffset];const auto directory=char(directoryByte&CharacterMask);
        const auto atomDefault=same(image.geometry().formatId,AcornAtomDos) && directory==Space;
        FileSystemEntry entry;entry.name=directory==RootDirectory || atomDefault?leaf:std::string(1,directory)+Dot+leaf;
        entry.kind=EntryKind::File;entry.size=fields.length;entry.storageReference=fields.startSector;
        entry.rawAttributes=(directoryByte&LockedBit)!=0?1:0;entry.metadataValid=catalogueValid;
        if(!catalogueValid)entry.dataValid=false;
        entry.comment=describeAddresses(fields.load,fields.execute);
        entry.content=std::vector<std::uint8_t>(fields.length);
        const auto sectors=(fields.length+SectorSize-1)/SectorSize;used+=sectors;
        for(std::uint32_t sector=0;sector<sectors;++sector) {
            checkCancellation(token);
            const auto logical=std::uint64_t(fields.startSector)+sector;
            const auto block=logical<total?image.block(logical,token):nullptr;
            if(!block) {
                result.warnings.push_back({DiagnosticCode::BbcFileOutside,{entry.name,std::to_string(fields.startSector),std::to_string(fields.length)}});
                entry.metadataValid=false;continue;
            }
            const auto output=sector*SectorSize;const auto length=std::min(SectorSize,fields.length-output);
            if(block->data.size()<length) {
                entry.metadataValid=false;
                entry.diagnostics.push_back({DiagnosticCode::SectorInvalidBlockSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(length)}});
                continue;
            }
            std::copy_n(block->data.begin(),length,entry.content->begin()+output);
            if(!validIntegrity(*block)) {
                entry.metadataValid=false;entry.dataValid=false;
                entry.diagnostics.push_back({DiagnosticCode::BbcIntegrityInvalid,{std::to_string(logical)}});
            }
        }
        result.entries.push_back(std::move(entry));
    }
    const auto key=[](std::string value){for(auto& c:value)if(c>=LowerFirst && c<=LowerLast)c=char(c-CaseDelta);return value;};
    std::stable_sort(result.entries.begin(),result.entries.end(),[&](const auto& a,const auto& b){return key(a.name)<key(b.name);});
    result.capacity=total*SectorSize;result.freeBytes=(total>used?total-used:0)*SectorSize;result.freeSpaceKnown=catalogueValid;
    return result;
}
}
bool canRead(const SectorImage& image,std::stop_token token){return Internal::twoSided(image)?Internal::canReadSides(image,token):Internal::canReadSingle(Internal::CatalogueView(image),token);}
FileSystemVolume read(const SectorImage& image,std::stop_token token){checkCancellation(token);return Internal::twoSided(image)?Internal::readSides(image,token):Internal::readSingle(Internal::CatalogueView(image),token);}
}
