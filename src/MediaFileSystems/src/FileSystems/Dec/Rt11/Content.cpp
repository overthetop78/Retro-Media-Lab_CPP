#include "Internal.h"
#include <bit>
namespace RetroMediaLab::MediaFileSystems::Rt11::Internal {
using namespace Constants;
Pair pair(const SectorImage& image,std::int32_t first,std::stop_token token){
    checkCancellation(token);Pair result;result.bytes.resize(BlockSize*SegmentBlockCount);const auto second=std::bit_cast<std::int32_t>(std::uint32_t(first)+1);const auto a=first>=0?image.block(std::uint64_t(first),token):nullptr,b=second>=0?image.block(std::uint64_t(second),token):nullptr;result.firstPresent=bool(a);result.secondPresent=bool(b);result.firstValid=a&&a->data.size()==BlockSize;result.secondValid=b&&b->data.size()==BlockSize;if(result.firstValid)std::copy(a->data.begin(),a->data.end(),result.bytes.begin());if(result.secondValid)std::copy(b->data.begin(),b->data.end(),result.bytes.begin()+BlockSize);return result;
}
Content content(const SectorImage& image,std::int32_t first,std::int32_t count,std::stop_token token){
    checkCancellation(token);Content result;if(count<0||std::uint64_t(count)>image.blockCount())return result;if(std::uint64_t(count)>MaximumStreamSize/BlockSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);result.bytes.resize(std::size_t(count)*BlockSize);
    for(std::int32_t index=0;index<count;++index){checkCancellation(token);const auto logical=std::bit_cast<std::int32_t>(std::uint32_t(first)+std::uint32_t(index));const auto block=logical>=0?image.block(std::uint64_t(logical),token):nullptr;if(!block||block->data.size()!=BlockSize)result.missing.push_back(logical);else std::copy(block->data.begin(),block->data.end(),result.bytes.begin()+std::size_t(index)*BlockSize);}result.valid=result.missing.empty();return result;
}
}
