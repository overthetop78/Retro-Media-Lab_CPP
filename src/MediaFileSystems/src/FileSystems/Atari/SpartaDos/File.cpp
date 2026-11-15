#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::SpartaDos::Internal {
using namespace Constants;
File file(const SectorImage& image,std::int32_t first,std::int32_t byteLength,std::stop_token token){
    checkCancellation(token);if(byteLength<0||std::uint64_t(byteLength)>image.capacity())throw MediaError(DiagnosticCode::SpartaDirectoryLength,{std::to_string(first),std::to_string(byteLength)});File result;if(byteLength==0)return result;const auto size=image.geometry().blockSize;if(size==0)throw MediaError(DiagnosticCode::SpartaDirectoryLength,{std::to_string(first),std::to_string(byteLength)});const auto count=(std::uint64_t(byteLength)+size-1)/size;if(count>std::uint64_t(std::numeric_limits<std::int32_t>::max())||count*size>MaximumStreamSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);const auto map=allocation(image,first,std::int32_t(count),token);result.bytes.reserve(std::size_t(count*size));std::size_t allocated=0;
    for(auto number:map.sectors){checkCancellation(token);if(number==0){result.bytes.resize(result.bytes.size()+size,0);result.sparse=true;continue;}const auto block=sector(image,number,token);if(block->data.size()<size)throw MediaError(DiagnosticCode::SpartaMissingSector,{std::to_string(number)});result.bytes.insert(result.bytes.end(),block->data.begin(),block->data.begin()+size);++allocated;}
    if(result.bytes.size()>std::size_t(byteLength))result.bytes.resize(std::size_t(byteLength));result.occupied=std::uint64_t(map.maps+allocated)*size;return result;
}
}
