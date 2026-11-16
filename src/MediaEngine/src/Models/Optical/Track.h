#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Representations.h>
#include <map>
#include <RetroMediaLab/MediaEngine/OpticalTrackMode.h>
namespace RetroMediaLab::MediaEngine {
class OpticalTrackIndex final {
public:
    RETROMEDIALAB_MEDIAENGINE_API OpticalTrackIndex(std::int32_t number,std::int64_t relativeSector);
    std::int32_t number()const noexcept{return number_;}
    std::int64_t relativeSector()const noexcept{return relative_;}
private:std::int32_t number_;std::int64_t relative_;
};
struct OpticalMetadataLess {RETROMEDIALAB_MEDIAENGINE_API bool operator()(const std::string&,const std::string&)const;};
using OpticalMetadata=std::map<std::string,std::string,OpticalMetadataLess>;
struct OpticalTrackOptions {
    std::int32_t sessionNumber=0,trackNumber=0;
    OpticalTrackMode mode=OpticalTrackMode::Audio;
    std::int64_t firstSector=0,sectorCount=0;
    std::int32_t storedSectorSize=0,userDataOffset=0,userDataLength=0;
    std::shared_ptr<const MediaFileSystems::RandomAccessData> dataSource;
    std::int64_t sourceOffset=0;
    std::vector<OpticalTrackIndex> indexes;
    std::int64_t pregapSectors=0,postgapSectors=0;
    std::vector<std::string> flags;
    std::optional<std::string> catalogNumber,isrc;
    std::shared_ptr<const MediaFileSystems::RandomAccessData> subchannelSource;
    std::int64_t subchannelSourceOffset=0;
    std::int32_t subchannelBytesPerSector=0;
    std::int64_t storedPregapSectors=0;
    std::optional<std::int64_t> storedPregapSourceOffset;
    std::int32_t subchannelStride=0;
    OpticalMetadata metadata;
};
class OpticalTrack final:public MediaFileSystems::OpticalTrack {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit OpticalTrack(OpticalTrackOptions);
    const OpticalTrackOptions& options()const noexcept{return *state_;}
    std::int32_t sessionNumber()const override{return state_->sessionNumber;}
    std::int32_t trackNumber()const override{return state_->trackNumber;}
    std::int64_t firstSector()const override{return state_->firstSector;}
    std::int64_t sectorCount()const override{return state_->sectorCount;}
    std::int32_t userDataLength()const override{return state_->userDataLength;}
    bool isAudio()const override{return state_->mode==OpticalTrackMode::Audio;}
    // Destination must remain alive until completion. This overload also accepts stored pregap sectors.
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> readStoredSectorAsync(std::int64_t,MediaFileSystems::MutableBytes,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readStoredSectorAsync(std::int64_t,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readUserDataAsync(std::int64_t,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::optional<std::vector<std::uint8_t>>> readSubchannelAsync(std::int64_t,std::stop_token={})const;
private:std::shared_ptr<const OpticalTrackOptions> state_;
};
}
