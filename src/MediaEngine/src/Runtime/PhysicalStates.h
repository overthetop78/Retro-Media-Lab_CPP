#pragma once
#include "DiskState.h"
#include <tuple>
#include "../Models/Flux/Representation.h"
#include "../Formats/HardDisk/MfmEmu/Representation.h"
namespace RetroMediaLab::MediaEngine {
struct RuntimeDisk::FluxState final:State {
    using Address=std::pair<std::int32_t,std::int32_t>;
    std::map<Address,Flux::ProtectedTrack> tracks;
    std::map<Address,FluxTrackData> cache;
    std::int64_t cylinders_=0;std::int32_t heads_=0;std::size_t revolutionCount=1;bool changed=false;
    FluxState(std::shared_ptr<RuntimeDetails::Services>,std::shared_ptr<const MediaDocument>,bool,bool);
    ~FluxState()noexcept override;
    std::int32_t size()const override{return 0;}std::int64_t count()const override{return 0;}
    std::int64_t cylinders()const override{return cylinders_;}std::int32_t heads()const override{return heads_;}std::int32_t sectorsPerTrack()const override{return 0;}
    bool dirty()const override{return changed;}void clear()noexcept override{cache.clear();tracks.clear();changed=false;}
    void validateAddress(std::int32_t,std::int32_t)const;
    FluxTrackData readTrack(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,const Attributes&)override;
    void writeTrackWindow(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::span<const std::uint32_t>,const Attributes&,bool)override;
    void flush(std::stop_token)override;
};
struct RuntimeDisk::MfmState final:State {
    std::shared_ptr<const MfmEmu::Representation> representation;
    std::optional<FluxTrackData> cached;std::tuple<std::int32_t,std::int32_t,std::uint32_t> cachedAddress{};bool changed=false;
    MfmState(std::shared_ptr<RuntimeDetails::Services>,std::shared_ptr<const MediaDocument>,bool,bool);
    ~MfmState()noexcept override;
    std::int32_t size()const override{alive();return representation->sectorReader()->sectorSize();}
    std::int64_t count()const override{alive();return representation->sectorReader()->sectorCount();}
    std::int64_t cylinders()const override{return representation->cylinders();}std::int32_t heads()const override{return representation->heads();}
    std::int32_t sectorsPerTrack()const override{alive();return representation->sectorReader()->sectorsPerTrack();}
    bool dirty()const override{return changed;}void clear()noexcept override{cached.reset();changed=false;}
    std::optional<std::int64_t> findSector(std::int32_t,std::int32_t,std::int32_t)const override;
    std::vector<std::int64_t> trackSectors(std::int32_t,std::int32_t)const override;
    MediaDiskSector read(std::int64_t,std::stop_token)override;
    void write(std::int64_t,std::span<const std::uint8_t>,std::optional<bool>)override;
    void updateMetadata(const std::map<std::string,std::string>&)override;
    void prepareBlockFormat(const HardDiskGeometry&)override;
    FluxTrackData readTrack(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,const Attributes&)override;
    void writeTrackWindow(std::int32_t,std::int32_t,std::string_view,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::span<const std::uint32_t>,const Attributes&,bool)override;
    void flush(std::stop_token)override;
};
}
