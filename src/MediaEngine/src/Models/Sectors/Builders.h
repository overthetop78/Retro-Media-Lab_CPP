#pragma once
#include "SectorImages.h"
namespace RetroMediaLab::MediaEngine {
enum class SectorNumbering {ZeroBased,OneBased};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> linearSectorImage(std::shared_ptr<const MediaFileSystems::RandomAccessData>,MediaFileSystems::SectorGeometry,SectorNumbering=SectorNumbering::ZeroBased,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> regularSectorImage(MediaFileSystems::Bytes,MediaFileSystems::SectorGeometry,std::int32_t firstSectorNumber=0,std::int32_t allowedTrailingBytes=0,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> ibmRawSectorImage(std::shared_ptr<const MediaFileSystems::RandomAccessData>,MediaFileSystems::SectorGeometry,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> macintoshSectorImage(MediaFileSystems::Bytes,std::string formatId,std::int32_t heads,std::stop_token={});
struct DenseSectorImage {std::vector<std::uint8_t> data;std::vector<std::int32_t> missingBlocks;};
RETROMEDIALAB_MEDIAENGINE_API DenseSectorImage denseSectorImage(std::span<const MediaFileSystems::SectorBlock>,std::int32_t count,std::int32_t blockSize,std::stop_token={});
}
