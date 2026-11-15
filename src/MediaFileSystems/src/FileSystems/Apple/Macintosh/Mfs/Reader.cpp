#include "Internal.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::MacMfs {
using namespace Constants;
bool canRead(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(image.geometry().blockSize!=SectorSize)return false;const auto block=image.block(MasterBlock,token);return block&&block->data.size()>=MinimumMdb&&Binary::big<std::uint16_t>(block->data,0)==Signature;
}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);FileSystemVolume volume;const auto information=Internal::readBlocks(image,MasterBlock,VolumeBlocks,Internal::BlockContext::Volume,volume.warnings,token);const auto& mdb=information.bytes;
    if(!information.valid||mdb.size()<MinimumMdb||Binary::big<std::uint16_t>(mdb,0)!=Signature){const auto signature=mdb.size()>=sizeof(std::uint16_t)?Binary::big<std::uint16_t>(mdb,0):0;throw MediaError(DiagnosticCode::MacInvalidVolume,{std::string(System),QString::number(signature,HexBase).rightJustified(HexWordWidth,QChar(HexZero)).toUpper().toStdString()});}
    const auto directoryStart=Binary::big<std::uint16_t>(mdb,DirectoryStart),directoryLength=Binary::big<std::uint16_t>(mdb,DirectoryLength),allocationCount=Binary::big<std::uint16_t>(mdb,AllocationCount);
    const auto allocationSize=Binary::big<std::uint32_t>(mdb,AllocationSize);if(allocationSize==0||allocationSize%SectorSize)throw MediaError(DiagnosticCode::MacAllocationSize,{std::string(System),std::to_string(allocationSize),std::to_string(SectorSize)});
    const auto allocationStart=Binary::big<std::uint16_t>(mdb,AllocationStart),freeCount=Binary::big<std::uint16_t>(mdb,FreeAllocationCount);
    volume.name=Macintosh::pascalString(mdb,VolumeName,MaximumVolumeName);const auto map=Internal::AllocationMap::decode(Bytes(mdb).subspan(MapOffset,MapLength),allocationCount);
    volume.entries=Internal::directory(image,directoryStart,directoryLength,map,allocationStart,allocationSize,volume.warnings,token);volume.freeBytes=freeCount<=allocationCount?std::uint64_t(freeCount)*allocationSize:0;
    if(freeCount>allocationCount)volume.warnings.push_back({DiagnosticCode::MacFreeCount,{std::string(System),std::to_string(freeCount),std::to_string(allocationCount)}});
    volume.fileSystemId=Id;volume.fileSystemDisplayName=Id;volume.capacity=image.capacity();volume.created=Macintosh::fromSeconds(Binary::big<std::uint32_t>(mdb,Created));volume.modified=Macintosh::fromSeconds(Binary::big<std::uint32_t>(mdb,Modified));return volume;
}
}
