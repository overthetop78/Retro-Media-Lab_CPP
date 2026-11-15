#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Internal {
namespace {
bool side(const Image& image,std::int32_t track,std::int32_t number,std::uint32_t count,std::size_t offset,std::size_t stride,std::int32_t& free,std::vector<Diagnostic>& warnings) {
    const auto read=sector(image,track,number);const auto length=read.block?read.block->data.size():0;
    if(read.status!=SectorStatus::Success||offset+std::uint64_t(count)*stride>length) {
        if(read.status==SectorStatus::Truncated||read.status==SectorStatus::Success)warnings.push_back({DiagnosticCode::CbmBamTruncated,{std::to_string(track),std::to_string(number),std::to_string(length)}});
        else warnings.push_back({DiagnosticCode::CbmBamMissing,{std::to_string(track),std::to_string(number)}});
        return false;
    }
    for(std::uint32_t index=0;index<count;++index){checkCancellation(image.token);free+=read.block->data[offset+index*stride];}return true;
}
}
std::optional<std::int32_t> freeBlocks(const Image& image,bool d81,std::vector<Diagnostic>& warnings) {
    using namespace Constants;checkCancellation(image.token);std::int32_t free=0;
    if(d81) {
        for(const auto sector:BamSectors1581)if(!side(image,HeaderTrack1581,sector,BamEntries1581,BamOffset1581,BamSize1581,free,warnings))return {};
    }else {
        const auto tracks=image.source.geometry().cylinders;
        if(!side(image,HeaderTrack1541,HeaderSector1541,tracks,BamOffset1541,BamSize1541,free,warnings))return {};
        if(image.source.geometry().heads==Commodore::Constants::TwoSides&&!side(image,HeaderTrack1541+std::int32_t(tracks),HeaderSector1541,tracks,BamOffset1541,BamSize1541,free,warnings))return {};
    }
    return free;
}
}
