#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine {
struct SourceSectorMetadata {std::int64_t firstSectorNumber=1;std::optional<std::int64_t> recordedHead;bool trustCompletePayload=false;std::function<MediaFileSystems::SectorAddress(std::uint64_t)> addressOf;};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> sparseSectorImage(MediaFileSystems::SectorGeometry, std::vector<MediaFileSystems::SectorBlock>, MediaFileSystems::SectorImageOptions = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> sourceSectorImage(std::shared_ptr<const MediaFileSystems::RandomAccessData>, MediaFileSystems::SectorGeometry, std::uint64_t offset = 0, MediaFileSystems::SectorImageOptions = {}, SourceSectorMetadata = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> relabelSectorImage(std::shared_ptr<const MediaFileSystems::SectorImage>, std::string formatId);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> imageFromWritePlan(const MediaFileSystems::SectorWritePlan&);
}
