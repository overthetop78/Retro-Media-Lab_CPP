#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos {
bool canRead(const SectorImage& source,std::stop_token token){return Internal::recognize({source,token}).has_value();}
FileSystemVolume read(const SectorImage& source,std::stop_token token) {
    checkCancellation(token);Internal::Image image{source,token};const auto recognized=Internal::recognize(image);
    if(!recognized)throw MediaError(DiagnosticCode::CbmUnsupported,{source.geometry().formatId});
    FileSystemVolume volume;volume.name=Petscii::decode(Bytes(recognized->header).subspan(recognized->layout.nameOffset,Constants::NameLength));
    volume.entries=Internal::directory(image,recognized->directoryTrack,recognized->directorySector,volume.warnings);
    const auto free=Internal::freeBlocks(image,recognized->layout.d81,volume.warnings);volume.freeSpaceKnown=free.has_value();volume.freeBytes=free?std::uint64_t(*free)*Constants::SectorSize:0;
    volume.fileSystemId=Constants::FileSystemId;volume.fileSystemDisplayName=Constants::FileSystemId;volume.capacity=source.capacity();return volume;
}
}
