#include "DiskState.h"
#include "../Formats/HardDisk/Adaptec/Codec.h"
#include "../Formats/HardDisk/Adaptec/Constants.h"
#include "../Formats/HardDisk/Acorn/Descriptor.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include <array>
#include <set>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace C=Adaptec::Constants;namespace A=AcornWinchester::Constants;namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeTrackProjection);}
Flux::NanosecondRevolution neutral(const FluxRevolutionData& r){return {r.indexTimeNanoseconds(),std::vector<std::uint32_t>(r.intervalsNanoseconds().begin(),r.intervalsNanoseconds().end())};}
bool readable(const BlockImage& image,std::int64_t offset,std::int64_t size){const auto end=offset+size;for(const auto& range:image.ranges()){if(range.address()+range.length()<=offset)continue;if(range.address()>offset||range.kind()==DataRangeKind::Unavailable)return false;offset=std::min(end,range.address()+range.length());if(offset==end)return true;}return false;}
}
void RuntimeDisk::State::writeWinchesterParameters(std::span<std::uint8_t> flags){
    const auto found=document->metadata().find(std::string(A::DescriptorKey));if(found==document->metadata().end())invalid();
    const auto descriptor=AcornWinchester::decodeDescriptor(found->second);if(!AcornWinchester::tryWritePhysicalParameters(descriptor,blocks()->capacity(),flags))invalid();
}
FluxTrackData RuntimeDisk::State::readWinchesterTrack(std::int32_t cylinder,std::int32_t head,std::uint32_t period){
    const auto pending=updated();const auto* image=dynamic_cast<const BlockImage*>(&pending->representation());if(!image||!image->geometry())invalid();
    const auto& geometry=*image->geometry();const auto count=geometry.sectorsPerTrack(),size=image->logicalBlockSize();const auto first=(std::int64_t(cylinder)*geometry.heads()+head)*count;
    const bool sequential=(size==C::MinimumBlockSize&&count==A::SmallSequentialSectors)||(size==C::MediumBlockSize&&count==A::MediumSequentialSectors);
    std::vector<M::SectorBlock> sectors(static_cast<std::size_t>(count));std::vector<bool> used(static_cast<std::size_t>(count));std::int32_t position=0;
    for(std::int32_t logical=0;logical<count;++logical){
        while(used[static_cast<std::size_t>(position)])if(++position==count)position=0;const auto address=first+logical;RuntimeDetails::coordinate(address);
        M::SectorBlock sector;sector.logicalBlock=static_cast<std::uint64_t>(address);sector.address={cylinder,head,address};
        if(readable(*image,address*size,size)){sector.data.resize(static_cast<std::size_t>(size));image->readExactly(address*size,sector.data);}
        sectors[static_cast<std::size_t>(position)]=std::move(sector);used[static_cast<std::size_t>(position)]=true;position+=sequential?1:C::DefaultInterleave;if(position>=count)position-=count;
    }
    std::array<std::uint8_t,C::ParameterFlagCount> flags{};const bool initial=cylinder==0&&head==0;if(initial)writeWinchesterParameters(flags);
    const auto r=Adaptec::encode(sectors,sequential,period,size,initial?std::span<const std::uint8_t>(flags):std::span<const std::uint8_t>{});return FluxTrackData({FluxRevolutionData(r.indexTime,r.intervals)});
}
void RuntimeDisk::State::writeWinchesterWindow(std::int32_t cylinder,std::int32_t head,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,bool ending){
    if(cylinder<0||head<0||cylinder>=cylinders()||head>=heads()||!period)throw MediaError(DiagnosticCode::RuntimeTrackWindow);
    const auto original=duration>=period?FluxTrackData({FluxRevolutionData(period,{})}):readTrack(cylinder,head,C::EncodingId,C::CellNanoseconds,period,{});
    auto changed=replaceFluxWindow(original.revolutions().front(),start,duration,offsets,ending);if(!duration)return;const TrackAddress address{cylinder,head};
    writtenTracks.insert_or_assign(address,FluxTrackData({std::move(changed)}));documentChanged=true;
    try{projectWinchesterTrack(cylinder,head);winchesterProjectionErrors.erase(address);}catch(const MediaError&){winchesterProjectionErrors.insert_or_assign(address,std::current_exception());}
}
void RuntimeDisk::State::projectWinchesterTrack(std::int32_t cylinder,std::int32_t head){
    const auto& track=writtenTracks.at({cylinder,head});const auto fields=Adaptec::decode(neutral(track.revolutions().front()),cylinder,head,size());
    const auto first=(std::int64_t(cylinder)*heads()+head)*sectorsPerTrack();if(fields.size()!=static_cast<std::size_t>(sectorsPerTrack()))invalid();
    std::set<std::int64_t> seen;std::array<std::uint8_t,C::ParameterFlagCount> flags{};const bool initial=cylinder==0&&head==0;std::size_t anchor=fields.size();
    if(initial){writeWinchesterParameters(flags);for(std::size_t i=0;i<fields.size();++i)if(fields[i].sector.address.number==0){anchor=i;break;}if(anchor==fields.size())invalid();}
    for(std::size_t i=0;i<fields.size();++i){const auto& f=fields[i];const auto& s=f.sector;const auto position=initial?(i+fields.size()-anchor)%fields.size():C::ParameterFlagCount;const auto expected=position<flags.size()?flags[position]:0;
        if(s.address.number<first||s.address.number>=first+sectorsPerTrack()||!seen.insert(s.address.number).second||s.data.size()!=static_cast<std::size_t>(size())||s.headerIntegrityValid!=true||s.dataIntegrityValid!=true||f.id.size()<=C::FlagsOffset||
            (f.id[C::AddressOffset]&C::DefectiveAddressByteMask)!=0||(f.id[C::FlagsOffset]&C::DefectCountMask)!=expected||((f.id[C::FlagsOffset]&C::LastSectorFlag)!=0)!=(s.address.number==first+sectorsPerTrack()-1))invalid();
    }
    for(const auto& f:fields)changes[static_cast<std::int64_t>(f.sector.logicalBlock)]=f.sector.data;
}
MediaDiskSector RuntimeDisk::State::readWrittenWinchesterSector(std::int64_t index,std::int32_t cylinder,std::int32_t head){
    const auto fields=Adaptec::decode(neutral(writtenTracks.at({cylinder,head}).revolutions().front()),cylinder,head,size());
    for(const auto& f:fields){const auto& b=f.sector;if(b.address.number==index&&b.headerIntegrityValid==true&&f.id.size()>C::AddressOffset&&(f.id[C::AddressOffset]&C::DefectiveAddressByteMask)==0&&b.data.size()==static_cast<std::size_t>(size())){
        MediaDiskSector result;result.index=static_cast<std::int64_t>(b.logicalBlock);result.data=b.data;result.cylinder=RuntimeDetails::coordinate(b.address.cylinder);result.head=RuntimeDetails::coordinate(b.address.head);result.number=RuntimeDetails::coordinate(b.address.number);result.integrityValid=b.integrityValid;result.diagnosticCode=b.diagnosticCode;result.recordedSizeCode=b.recordedSizeCode;result.headerIntegrityValid=b.headerIntegrityValid;result.dataIntegrityValid=b.dataIntegrityValid;result.deletedDataMark=b.deletedDataMark;result.recordedErrorCheckCode=b.recordedErrorCheckCode;result.recordedDataPrefix=b.recordedDataPrefix;return result;
    }}invalid();
}
}
