#include "WriterInternal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
using namespace Constants;
namespace {
void identity(MutableBytes sector,std::size_t offset,std::optional<std::size_t> dosOffset={},Bytes dos={}){
    std::copy(DiskIdentity.begin(),DiskIdentity.end(),sector.begin()+offset);
    if(dosOffset)std::copy(dos.begin(),dos.end(),sector.begin()+*dosOffset);
}
}
void Builder::bam1541(MutableBytes bam,std::int32_t side){
    for(std::int32_t track=1;track<=std::int32_t(geometry_.geometry.cylinders);++track){
        checkCancellation(token_);const auto global=track+side*std::int32_t(geometry_.geometry.cylinders);
        const auto offset=BamOffset1541+(track-1)*BamSize1541;std::uint8_t count=0;
        for(std::int32_t sector=0;sector<Commodore::sectors1541(track);++sector){
            if(!free_[geometry_.logical(global,sector)])continue;
            ++count;bam[offset+1+sector/BitsPerByte]|=std::uint8_t(1<<(sector%BitsPerByte));
        }
        bam[offset]=count;
    }
}
void Builder::system1541(){
    auto& header=sectors_[geometry_.logical(HeaderTrack1541,HeaderSector1541)];
    address(header,0,directories_.front());header[LinkLength]=Signature1541;
    const auto name=nameBytes(plan_.volumeName);std::copy(name.begin(),name.end(),header.begin()+Name1541);
    identity(header,DiskId1541,DosType1541,DosIdentity1541);bam1541(header,0);
    if(geometry_.geometry.heads==Commodore::Constants::TwoSides){
        auto& second=sectors_[geometry_.logical(HeaderTrack1541+geometry_.geometry.cylinders,HeaderSector1541)];
        second[LinkLength]=Signature1541;bam1541(second,1);
    }
}
void Builder::bam1581(MutableBytes bam,std::int32_t first){
    for(std::int32_t index=0;index<BamEntries1581;++index){
        checkCancellation(token_);const auto offset=BamOffset1581+index*BamSize1581;std::uint8_t count=0;
        for(std::int32_t sector=0;sector<std::int32_t(geometry_.geometry.sectorsPerTrack);++sector){
            if(!free_[geometry_.logical(first+index,sector)])continue;
            ++count;bam[offset+1+sector/BitsPerByte]|=std::uint8_t(1<<(sector%BitsPerByte));
        }
        bam[offset]=count;
    }
}
void Builder::system1581(){
    auto& header=sectors_[geometry_.logical(HeaderTrack1581,HeaderSector1581)];
    address(header,0,directories_.front());header[LinkLength]=Signature1581;
    const auto name=nameBytes(plan_.volumeName);std::copy(name.begin(),name.end(),header.begin()+Name1581);
    identity(header,DiskId1581,DosType1581,DosIdentity1581);
    for(std::size_t side=0;side<BamSectors1581.size();++side){
        auto& bam=sectors_[geometry_.logical(HeaderTrack1581,BamSectors1581[side])];
        if(side==0)address(bam,0,geometry_.logical(HeaderTrack1581,BamSectors1581.back()));
        bam[LinkLength]=Signature1581;identity(bam,BamDiskId1581);bam1581(bam,std::int32_t(side)*BamEntries1581+1);
    }
}
void Builder::system(){if(geometry_.is1581())system1581();else system1541();}
}
