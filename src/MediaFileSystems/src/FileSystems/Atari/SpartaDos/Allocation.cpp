#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::SpartaDos::Internal {
using namespace Constants;
Allocation allocation(const SectorImage& image,std::int32_t first,std::int32_t count,std::stop_token token){
    checkCancellation(token);Allocation result;if(count<=0)return result;validateSector(first,image.blockCount());result.sectors.reserve(std::size_t(count));std::set<std::int32_t> visited;auto current=first;std::int32_t previous=0;
    while(current!=0&&result.sectors.size()<std::size_t(count)){
        checkCancellation(token);if(!visited.insert(current).second)throw MediaError(DiagnosticCode::SpartaMapLoop,{std::to_string(current)});const auto block=sector(image,current,token);const auto bytes=Bytes(block->data);if(bytes.size()<SectorMapDataOffset)throw MediaError(DiagnosticCode::SpartaMissingSector,{std::to_string(current)});const auto observed=Binary::little<std::uint16_t>(bytes,SectorMapPreviousOffset);if(observed!=previous)throw MediaError(DiagnosticCode::SpartaMapBackLink,{std::to_string(current),std::to_string(previous),std::to_string(observed)});
        for(std::size_t offset=SectorMapDataOffset;offset+SectorNumberSize<=bytes.size()&&result.sectors.size()<std::size_t(count);offset+=SectorNumberSize){const auto number=Binary::little<std::uint16_t>(bytes,offset);if(number!=0)validateSector(number,image.blockCount());result.sectors.push_back(number);}
        previous=current;current=Binary::little<std::uint16_t>(bytes,SectorMapNextOffset);if(current!=0)validateSector(current,image.blockCount());
    }
    if(result.sectors.size()<std::size_t(count))throw MediaError(DiagnosticCode::SpartaMissingSector,{std::to_string(0)});result.maps=visited.size();return result;
}
std::int32_t firstData(const SectorImage& image,std::int32_t first,std::stop_token token){const auto result=allocation(image,first,1,token);const auto number=result.sectors[0];if(number==0)throw MediaError(DiagnosticCode::SpartaSectorRange,{std::to_string(number),std::to_string(image.blockCount())});return number;}
}
