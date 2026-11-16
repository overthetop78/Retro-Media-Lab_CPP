#pragma once
#include "Export.h"
#include <cstdint>
namespace RetroMediaLab::MediaEngine {
class HardDiskGeometry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API HardDiskGeometry(std::int64_t cylinders,std::int32_t heads,std::int32_t sectorsPerTrack,std::int32_t sectorSize);
    std::int64_t cylinders()const noexcept{return cylinders_;}std::int32_t heads()const noexcept{return heads_;}std::int32_t sectorsPerTrack()const noexcept{return sectors_;}std::int32_t sectorSize()const noexcept{return size_;}
    std::int64_t logicalSectorCount()const noexcept{return count_;}std::int64_t capacity()const noexcept{return capacity_;}
private:
    std::int64_t cylinders_,count_,capacity_;std::int32_t heads_,sectors_,size_;
};
}
