#include "SectorExploration.h"
#include "../Models/Sectors/SectorImages.h"
namespace RetroMediaLab::MediaEngine {
MediaFileSystems::FileSystemVolume exploreSectorImage(const MediaFileSystems::SectorImage& image,std::stop_token token) {
    auto volume=MediaFileSystems::Fat12::read(image,token);
    MediaFileSystems::analyzeEntries(image.geometry().formatId,volume,token);
    return volume;
}
MediaFileSystems::FileSystemVolume exploreSectorSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,MediaFileSystems::SectorGeometry geometry,std::uint64_t offset,std::stop_token token) {
    const auto image=sourceSectorImage(std::move(source),std::move(geometry),offset);
    return exploreSectorImage(*image,token);
}
}
