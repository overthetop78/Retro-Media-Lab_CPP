#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::MacHfs {
using namespace Constants;
namespace {
std::shared_ptr<const SectorBlock> mdb(const SectorImage& image,std::stop_token token){checkCancellation(token);if(image.geometry().blockSize!=SectorSize)return {};auto block=image.block(MasterDirectoryBlock,token);return block&&block->data.size()>=MinimumMdbLength?block:nullptr;}
}
bool canRead(const SectorImage& image,std::stop_token token){const auto block=mdb(image,token);return block&&Binary::big<std::uint16_t>(block->data,0)==Signature;}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    const auto block=mdb(image,token);if(!block||Binary::big<std::uint16_t>(block->data,0)!=Signature){const auto observed=block?Binary::big<std::uint16_t>(block->data,0):0;throw MediaError(DiagnosticCode::MacInvalidVolume,{std::string(SystemName),QString::number(observed,HexBase).rightJustified(HexWordWidth,QChar(HexZero)).toUpper().toStdString()});}
    const auto& bytes=block->data;const auto count=Binary::big<std::uint16_t>(bytes,AllocationCountOffset),start=Binary::big<std::uint16_t>(bytes,AllocationStartOffset),free=Binary::big<std::uint16_t>(bytes,FreeAllocationCountOffset);const auto size=Binary::big<std::uint32_t>(bytes,AllocationSizeOffset);if(size==0||size%SectorSize)throw MediaError(DiagnosticCode::MacAllocationSize,{std::string(SystemName),std::to_string(size),std::to_string(SectorSize)});
    FileSystemVolume volume;volume.name=Macintosh::pascalString(bytes,VolumeNameOffset,MaximumVolumeNameLength);const auto length=Binary::big<std::uint32_t>(bytes,CatalogLengthOffset);const auto content=Internal::extents(image,Bytes(bytes).subspan(CatalogExtentsOffset,EmbeddedExtentsLength),start,size,length,token);
    for(auto missing:content.missing)volume.warnings.push_back({DiagnosticCode::HfsCatalogBlockMissing,{std::to_string(missing)}});
    if(content.remaining)volume.warnings.push_back({DiagnosticCode::HfsCatalogIncomplete,{std::to_string(std::uint64_t(length)-content.remaining),std::to_string(length)}});
    const auto records=Internal::catalog(content.content,image,start,size,volume.warnings,token);volume.entries=Internal::buildDirectory(records,volume.warnings,token);volume.capacity=std::uint64_t(count)*size;volume.freeBytes=free<=count?std::uint64_t(free)*size:0;
    if(free>count)volume.warnings.push_back({DiagnosticCode::MacFreeCount,{std::string(SystemName),std::to_string(free),std::to_string(count)}});
    volume.fileSystemId=Id;applyFileSystemDisplayName(volume,image.geometry());volume.created=Macintosh::fromSeconds(Binary::big<std::uint32_t>(bytes,CreatedOffset));volume.modified=Macintosh::fromSeconds(Binary::big<std::uint32_t>(bytes,ModifiedOffset));return volume;
}
}
