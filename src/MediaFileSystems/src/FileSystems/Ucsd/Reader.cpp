#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Ucsd {
using namespace Constants;
bool canRead(const SectorImage& image,std::stop_token token){checkCancellation(token);return Internal::sameFormat(image.geometry().formatId)&&image.geometry().blockSize==BlockSize&&Internal::header(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto header=Internal::header(image,token);if(!header){const auto probe=Internal::blocks(image,DirectoryBlock,1,token);throw MediaError(DiagnosticCode::UcsdUnrecognized,{std::to_string(DirectoryBlock),Internal::join(probe.missing)});}
    FileSystemVolume volume;const auto directory=Internal::blocks(image,DirectoryBlock,header->blocks,token);if(!directory.valid())volume.warnings.push_back({DiagnosticCode::UcsdMissingDirectory,{Internal::join(directory.missing)}});auto decoded=Internal::directory(image,*header,directory,volume.warnings,token);
    const auto free=decoded.valid&&std::size_t(header->total)>decoded.used.size()?std::size_t(header->total)-decoded.used.size():0;volume.name=header->name;volume.fileSystemId=Id;volume.fileSystemDisplayName=Id;volume.capacity=std::uint64_t(header->total)*BlockSize;volume.freeBytes=std::uint64_t(free)*BlockSize;volume.modified=header->date;volume.entries=std::move(decoded.entries);return volume;
}
}
