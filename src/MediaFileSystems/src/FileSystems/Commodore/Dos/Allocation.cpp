#include "WriterInternal.h"
#include <algorithm>
#include <cstdlib>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
using namespace Constants;
std::vector<std::int32_t> Builder::allocate(std::size_t count){
    checkCancellation(token_);if(count==0)return {};
    std::vector<std::int32_t> selected;
    for(std::int32_t block=0;block<geometry_.count;++block)if(free_[block])selected.push_back(block);
    std::stable_sort(selected.begin(),selected.end(),[&](auto left,auto right){return std::abs(geometry_.address(left).first-geometry_.center())<std::abs(geometry_.address(right).first-geometry_.center());});
    if(selected.size()<count)throw MediaError(DiagnosticCode::CbmWriteFull);
    selected.resize(count);for(auto block:selected)free_[block]=false;return selected;
}
std::optional<std::int32_t> Builder::tryAllocate(std::int32_t track,std::int32_t sector){
    std::int32_t block;
    try{block=geometry_.logical(track,sector);}
    catch(const MediaError& error){switch(error.diagnostic().code){case DiagnosticCode::CommodoreTrack:case DiagnosticCode::CommodoreTrackCount:case DiagnosticCode::CommodoreSide:case DiagnosticCode::CommodoreSector:case DiagnosticCode::CommodoreBlock:return {};default:throw;}}
    if(!free_[block])return {};free_[block]=false;return block;
}
void Builder::reserveSystem(){
    free_[geometry_.logical(geometry_.center(),0)]=false;
    if(geometry_.is1581()){for(auto sector:BamSectors1581)free_[geometry_.logical(HeaderTrack1581,sector)]=false;}
    else if(geometry_.geometry.heads==Commodore::Constants::TwoSides)free_[geometry_.logical(HeaderTrack1541+geometry_.geometry.cylinders,HeaderSector1541)]=false;
}
void Builder::allocateDirectories(){
    const auto count=std::max(std::size_t(1),(plan_.entries.size()+DirectorySlots-1)/DirectorySlots);
    for(std::size_t index=0;index<count;++index){
        checkCancellation(token_);const auto first=geometry_.is1581()?DirectorySector1581:DirectorySector1541;
        const auto selected=index<std::size_t(geometry_.count)?tryAllocate(geometry_.center(),first+std::int32_t(index)):std::nullopt;
        directories_.push_back(selected?*selected:allocate(1).front());
    }
}
void Builder::address(MutableBytes target,std::size_t offset,std::int32_t block) const{
    const auto [track,sector]=geometry_.address(block);target[offset]=std::uint8_t(track);target[offset+1]=std::uint8_t(sector);
}
}
