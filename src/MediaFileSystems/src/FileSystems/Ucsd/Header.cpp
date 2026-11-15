#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Ucsd::Internal {
using namespace Constants;
std::optional<Header> header(const SectorImage& image,std::stop_token token){
    const auto first=blocks(image,DirectoryBlock,1,token);if(!first.valid())return {};const auto order=detectOrder(first.bytes);if(!order)return {};Header result;result.order=*order;result.end=word(first.bytes,DirectoryEndOffset,*order);result.total=word(first.bytes,TotalBlocksOffset,*order);result.count=word(first.bytes,FileCountOffset,*order);result.name=name(Bytes(first.bytes).subspan(VolumeNameOffset,VolumeNameFieldLength),MaximumVolumeNameLength);
    if(!directoryEnd(result.end)||result.end>result.total||std::uint64_t(result.total)>image.blockCount()||result.count>MaximumFileCount||result.name.empty())return {};
    result.date=date(word(first.bytes,VolumeDateOffset,*order));result.blocks=result.end==LongDirectoryEnd?LongDirectoryBlockCount:ShortDirectoryBlockCount;return result;
}
}
