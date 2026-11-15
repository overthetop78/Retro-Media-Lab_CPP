#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive::Internal {
ReadResult data(const SectorImage& image,std::uint64_t offset,std::uint64_t length,std::stop_token token){
    checkCancellation(token);const auto size=image.geometry().blockSize;
    if(!size)throw MediaError(DiagnosticCode::InvalidBlockSize);
    if(offset>image.capacity()||length>image.capacity()-offset||length>Constants::MaximumRangeLength)
        throw MediaError(DiagnosticCode::AmigaFlatRange,{std::to_string(offset),std::to_string(length),std::to_string(image.capacity())});
    ReadResult result;result.bytes.resize(std::size_t(length));
    for(std::uint64_t destination=0;destination<length;){
        checkCancellation(token);const auto absolute=offset+destination,number=absolute/size,within=absolute%size,count=std::min(length-destination,std::uint64_t(size)-within);
        if(const auto block=image.block(number,token)){
            if(block->integrityValid!=true)result.invalid.push_back(number);
            if(within<block->data.size())std::copy_n(block->data.begin()+std::size_t(within),std::size_t(std::min(count,block->data.size()-within)),result.bytes.begin()+std::size_t(destination));
        }else result.missing.push_back(number);
        destination+=count;
    }
    return result;
}
}
