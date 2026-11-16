#pragma once
#include "Geometry.h"
#include <functional>
namespace RetroMediaLab::MediaEngine::Adaptec {
using TrackReader=std::function<std::vector<Flux::NanosecondRevolution>(std::int32_t,std::int32_t)>;
class SectorReader final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SectorReader(TrackReader,std::int32_t physicalCylinders,std::int32_t physicalHeads);
    RETROMEDIALAB_MEDIAENGINE_API ~SectorReader();
    RETROMEDIALAB_MEDIAENGINE_API Geometry geometry()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectorSize()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectorsPerTrack()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t sectorCount()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t capacity()const;
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::int64_t> findSector(std::int32_t cylinder,std::int32_t head,std::int32_t number)const;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int64_t> trackSectors(std::int32_t cylinder,std::int32_t head)const;
    RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorBlock readSector(std::int64_t,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API void readExactly(std::int64_t,MediaFileSystems::MutableBytes,std::stop_token={})const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> blockData(std::shared_ptr<SectorReader>);
}
