#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <tuple>
namespace RetroMediaLab::MediaFileSystems::Rt11 {
using namespace Constants;
bool canRead(const SectorImage& image,std::stop_token token){checkCancellation(token);if(!Internal::sameFormat(image.geometry().formatId)||image.geometry().blockSize!=BlockSize)return false;const auto block=image.block(HomeBlock,token);return block&&looksLike(block->data);}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto block=image.block(HomeBlock,token);if(!block||block->data.size()!=BlockSize)throw MediaError(DiagnosticCode::Rt11Home,{std::string{},std::to_string(0)});const auto bytes=Bytes(block->data);const auto signature=decodeAscii(bytes.subspan(SystemIdOffset,SystemIdLength));const auto first=Binary::little<std::uint16_t>(bytes,DirectoryBlockOffset);if(!canRead(image,token))throw MediaError(DiagnosticCode::Rt11Home,{signature,std::to_string(first)});
    FileSystemVolume volume;volume.name=decodeAscii(bytes.subspan(VolumeNameOffset,VolumeNameLength));auto directory=Internal::directory(image,first,token);for(const auto& entry:directory.entries)if(entry.modified){const auto& d=*entry.modified;if(!volume.modified||std::tie(d.year,d.month,d.day)>std::tie(volume.modified->year,volume.modified->month,volume.modified->day))volume.modified=d;}
    volume.fileSystemId=Id;applyFileSystemDisplayName(volume,image.geometry());volume.capacity=image.capacity();volume.freeBytes=directory.free*BlockSize;volume.entries=std::move(directory.entries);volume.warnings=std::move(directory.warnings);return volume;
}
}
