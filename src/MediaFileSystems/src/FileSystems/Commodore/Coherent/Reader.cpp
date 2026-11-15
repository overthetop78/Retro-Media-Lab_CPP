#include "Internal.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Coherent {
FileSystemVolume read(const SectorImage& source,std::stop_token token){
 using namespace Constants;checkCancellation(token);
 if(source.geometry().blockSize!=BlockSize)throw MediaError(DiagnosticCode::InvalidBlockSize);
 if(source.blockCount()>std::numeric_limits<std::uint64_t>::max()/BlockSize)throw MediaError(DiagnosticCode::CoherentInvalidSuperblock);
 Internal::Image image(source,token);if(!image.present(0,MinimumImageSize))throw MediaError(DiagnosticCode::CoherentMissingSuperblock);
 const auto header=image.bytes(0,MinimumImageSize);const auto count=validatedBlocks(header,image.blockCount()*BlockSize);const auto zoneEnd=Binary::little<std::uint16_t>(header,InodeZoneEndOffset);
 if(zoneEnd<MinimumInodeZoneEnd||zoneEnd>count)throw MediaError(DiagnosticCode::CoherentInodeZone,{std::to_string(zoneEnd),std::to_string(count)});
 FileSystemVolume volume;std::set<std::uint16_t> stack,visited;auto root=Internal::directory(image,zoneEnd,RootInode,stack,visited,volume.warnings,inodeDescription(RootInode),0);volume.entries=std::move(root.entries);
 volume.name=decodeName(Bytes(header).subspan(VolumeNameOffset,NameLength));if(volume.name==Placeholder||volume.name==DefaultName)volume.name.clear();
 volume.fileSystemId=FileSystemId;volume.fileSystemDisplayName=FileSystemId;volume.capacity=std::uint64_t(count)*BlockSize;volume.freeBytes=std::min(volume.capacity,std::uint64_t(canonicalUInt32(Bytes(header).subspan(FreeBlockCountOffset,CanonicalLength)))*BlockSize);volume.modified=decodeTime(canonicalUInt32(Bytes(header).subspan(ModifiedTimeOffset,CanonicalLength)));return volume;
}
}
