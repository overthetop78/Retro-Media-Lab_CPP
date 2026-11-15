#include "Internal.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
void copyBlocks(const BlockImage& image,const BlockOutput& output,std::stop_token token){std::vector<std::uint8_t> buffer(Blocks::CopyBufferSize);std::int64_t address=0;while(address<image.capacity()){const auto count=std::min(buffer.size(),std::size_t(image.capacity()-address));const auto part=M::MutableBytes(buffer).first(count);image.readExactly(address,part,token);M::checkCancellation(token);output(part);address+=std::int64_t(count);}}
void copyStoredRange(const DataRange& range,const BlockOutput& output,std::stop_token token){if(!range.source())throw M::MediaError(M::DiagnosticCode::BlockSourceNull);std::vector<std::uint8_t> buffer(Blocks::CopyBufferSize);std::int64_t completed=0;while(completed<range.length()){const auto count=std::min(buffer.size(),std::size_t(range.length()-completed));const auto part=M::MutableBytes(buffer).first(count);range.source()->readExactly(std::uint64_t(Blocks::add(range.sourceOffset(),completed)),part,token);M::checkCancellation(token);output(part);completed+=std::int64_t(count);}}
std::optional<std::vector<std::uint8_t>> readStoredRange(const DataRange* range,std::int64_t relative,std::int64_t length,std::stop_token token){
    if(!range||!range->source()||length<=0||length>std::numeric_limits<int>::max()||relative<0||relative>range->length()||length>range->length()-relative)return {};
    std::vector<std::uint8_t> bytes(std::size_t(length),std::uint8_t{});range->source()->readExactly(std::uint64_t(Blocks::add(range->sourceOffset(),relative)),bytes,token);return bytes;
}
std::optional<std::vector<std::uint8_t>> readStoredRange(const DataRange* range,std::stop_token token){return readStoredRange(range,0,range?range->length():0,token);}
}
