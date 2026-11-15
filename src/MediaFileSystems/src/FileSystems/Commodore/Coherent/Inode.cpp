#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Coherent::Internal {
Inode inode(Image& image,std::uint32_t zoneEnd,std::uint16_t number){
 if(!number)throw MediaError(DiagnosticCode::CoherentNullInode);
 const auto offset=std::uint64_t(BlockSize)*DirectoryInodeLength+(number-1)*InodeSize,limit=std::uint64_t(zoneEnd)*BlockSize;
 if(offset>limit||limit-offset<InodeSize||!image.present(offset,InodeSize))throw MediaError(DiagnosticCode::CoherentInodeOutside,{std::to_string(number),std::to_string(limit)});
 const auto value=image.bytes(offset,InodeSize);Inode result;result.mode=Binary::little<std::uint16_t>(value,ModeOffset);result.size=canonicalUInt32(Bytes(value).subspan(SizeOffset,CanonicalLength));result.modified=canonicalUInt32(Bytes(value).subspan(ModifiedOffset,CanonicalLength));
 for(std::size_t i=0;i<result.blocks.size();++i)result.blocks[i]=blockPointer(Bytes(value).subspan(PointersOffset+i*PointerSize,PointerSize));return result;
}
}
