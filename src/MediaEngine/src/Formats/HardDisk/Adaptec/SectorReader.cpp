#include "SectorReader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <mutex>
#include <tuple>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adaptec {
namespace {
bool usable(const Field& field){return field.sector.headerIntegrityValid==true&&(field.id[Constants::AddressOffset]&Constants::DefectiveAddressByteMask)==0;}
}
struct SectorReader::Impl {
    TrackReader readTrack;std::int32_t physicalCylinders,physicalHeads;mutable std::mutex mutex;Geometry shape;bool loaded=false;std::int64_t count{};std::optional<std::tuple<std::int32_t,std::int32_t,std::int32_t>> address;std::vector<Field> fields;
    Impl(TrackReader reader,std::int32_t cylinders,std::int32_t heads):readTrack(std::move(reader)),physicalCylinders(cylinders),physicalHeads(heads){}
    const std::vector<Field>& decodeTrack(std::int32_t cylinder,std::int32_t head,std::int32_t size,std::stop_token token={}){
        const auto key=std::tuple(cylinder,head,size);if(address&&*address==key)return fields;checkCancellation(token);const auto track=readTrack(cylinder,head);if(track.empty())throw MediaError(DiagnosticCode::AdaptecNoRevolution);auto decoded=decode(track.front(),cylinder,head,size,token);fields=std::move(decoded);address=key;return fields;
    }
    void ensureGeometry(){
        if(loaded)return;const auto value=readGeometry(decodeTrack(0,0,0));if(value&&value->cylinders<=physicalCylinders&&value->heads<=physicalHeads){shape=*value;for(const auto& field:decodeTrack(shape.cylinders-1,shape.heads-1,shape.sectorSize))if(usable(field)&&(field.id[Constants::FlagsOffset]&Constants::LastSectorFlag))count=std::max(count,field.sector.address.number+1);}loaded=true;
    }
};
SectorReader::SectorReader(TrackReader reader,std::int32_t cylinders,std::int32_t heads){if(!reader)throw MediaError(DiagnosticCode::AdaptecTrackReader);impl_=std::make_unique<Impl>(std::move(reader),cylinders,heads);}
SectorReader::~SectorReader()=default;
Geometry SectorReader::geometry()const{std::lock_guard lock(impl_->mutex);impl_->ensureGeometry();return impl_->shape;}
std::int32_t SectorReader::sectorSize()const{return geometry().sectorSize;}
std::int32_t SectorReader::sectorsPerTrack()const{return geometry().sectorsPerTrack;}
std::int64_t SectorReader::sectorCount()const{std::lock_guard lock(impl_->mutex);impl_->ensureGeometry();return impl_->count;}
std::int64_t SectorReader::capacity()const{std::lock_guard lock(impl_->mutex);impl_->ensureGeometry();return impl_->count*impl_->shape.sectorSize;}
std::optional<std::int64_t> SectorReader::findSector(std::int32_t cylinder,std::int32_t head,std::int32_t number)const{
    std::lock_guard lock(impl_->mutex);impl_->ensureGeometry();if(!impl_->shape.sectorSize)return {};std::optional<std::int64_t> found;for(const auto& field:impl_->decodeTrack(cylinder,head,impl_->shape.sectorSize))if(usable(field)&&field.sector.address.number==number){if(found)return {};found=std::int64_t(field.sector.logicalBlock);}return found;
}
std::vector<std::int64_t> SectorReader::trackSectors(std::int32_t cylinder,std::int32_t head)const{
    std::lock_guard lock(impl_->mutex);impl_->ensureGeometry();std::vector<std::int64_t> result;if(!impl_->shape.sectorSize)return result;for(const auto& field:impl_->decodeTrack(cylinder,head,impl_->shape.sectorSize))if(usable(field))result.push_back(std::int64_t(field.sector.logicalBlock));return result;
}
MediaFileSystems::SectorBlock SectorReader::readSector(std::int64_t index,std::stop_token token)const{
    std::lock_guard lock(impl_->mutex);checkCancellation(token);impl_->ensureGeometry();if(index<0||index>=impl_->count)throw MediaError(DiagnosticCode::AdaptecSectorAddress,{std::to_string(index)});
    const auto first=index/impl_->shape.sectorsPerTrack,tracks=std::int64_t(impl_->shape.cylinders)*impl_->shape.heads;
    for(auto track=first;track<tracks;++track){checkCancellation(token);std::optional<MediaFileSystems::SectorBlock> found;
        for(const auto& field:impl_->decodeTrack(std::int32_t(track/impl_->shape.heads),std::int32_t(track%impl_->shape.heads),impl_->shape.sectorSize,token))if(usable(field)&&field.sector.logicalBlock==std::uint64_t(index)){if(found)throw MediaError(DiagnosticCode::AdaptecSectorIdentity,{std::to_string(index)});found=field.sector;}
        if(found)return std::move(*found);
    }
    throw MediaError(DiagnosticCode::AdaptecSectorIdentity,{std::to_string(index)});
}
void SectorReader::readExactly(std::int64_t address,MediaFileSystems::MutableBytes destination,std::stop_token token)const{
    checkCancellation(token);const auto length=capacity();if(address<0||address>length||destination.size()>std::uint64_t(length-address))throw MediaError(DiagnosticCode::AdaptecReadRange);const auto size=sectorSize();
    while(!destination.empty()){checkCancellation(token);const auto sector=readSector(address/size,token);if(sector.integrityValid!=true||sector.data.size()!=std::size_t(size))throw MediaError(DiagnosticCode::AdaptecSectorData,{std::to_string(address/size)});const auto offset=std::size_t(address%size),count=std::min(destination.size(),std::size_t(size)-offset);std::copy_n(sector.data.begin()+offset,count,destination.begin());destination=destination.subspan(count);address+=std::int64_t(count);}
}
namespace {
class BlockData final:public MediaFileSystems::RandomAccessData {
    std::shared_ptr<SectorReader> reader_;std::uint64_t length_;
public:
    explicit BlockData(std::shared_ptr<SectorReader> reader):reader_(std::move(reader)),length_(std::uint64_t(reader_->capacity())){}
    std::uint64_t length()const noexcept override{return length_;}
    void readExactly(std::uint64_t address,MediaFileSystems::MutableBytes bytes,std::stop_token token)const override{if(address>length_||bytes.size()>length_-address)throw MediaError(DiagnosticCode::AdaptecReadRange);reader_->readExactly(std::int64_t(address),bytes,token);}
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> blockData(std::shared_ptr<SectorReader> reader){if(!reader)throw MediaError(DiagnosticCode::AdaptecTrackReader);return std::make_shared<const BlockData>(std::move(reader));}
}
