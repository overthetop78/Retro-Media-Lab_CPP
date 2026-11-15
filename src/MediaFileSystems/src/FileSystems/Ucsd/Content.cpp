#include "Internal.h"
#include <bit>
namespace RetroMediaLab::MediaFileSystems::Ucsd::Internal {
using namespace Constants;
Blocks blocks(const SectorImage& image,std::int32_t first,std::int32_t count,std::stop_token token){
    checkCancellation(token);if(count<0||std::uint64_t(count)>MaximumStreamSize/BlockSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);Blocks result;result.bytes.resize(std::size_t(count)*BlockSize);result.present.resize(count,false);
    for(std::int32_t index=0;index<count;++index){checkCancellation(token);const auto logical=std::bit_cast<std::int32_t>(std::uint32_t(first)+std::uint32_t(index));const auto block=logical>=0?image.block(std::uint64_t(logical),token):nullptr;if(!block||block->data.size()!=BlockSize){result.missing.push_back(logical);continue;}std::copy(block->data.begin(),block->data.end(),result.bytes.begin()+std::size_t(index)*BlockSize);result.present[index]=true;}return result;
}
Content content(const SectorImage& image,std::int32_t first,std::int32_t end,std::int32_t lastBytes,std::string_view name,std::vector<Diagnostic>& warnings,std::stop_token token){
    checkCancellation(token);const auto count=std::int64_t(end)-first;Content result;if(count<=0){result.valid=count==0&&lastBytes==0;return result;}if(lastBytes<0||lastBytes>BlockSize){warnings.push_back({DiagnosticCode::UcsdLastBytes,{std::string(name),std::to_string(lastBytes)}});return result;}
    if(count>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::ReadOutOfBounds);result.size=std::uint64_t(count-1)*BlockSize+std::uint64_t(lastBytes==0?BlockSize:lastBytes);auto read=blocks(image,first,std::int32_t(count),token);result.valid=read.valid();result.missing=std::move(read.missing);result.bytes=std::move(read.bytes);result.bytes.resize(std::size_t(std::min(result.size,std::uint64_t(result.bytes.size()))));if(!result.valid)warnings.push_back({DiagnosticCode::UcsdMissingBlocks,{std::string(name),join(result.missing)}});return result;
}
}
