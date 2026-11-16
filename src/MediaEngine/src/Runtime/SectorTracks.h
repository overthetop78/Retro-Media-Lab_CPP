#pragma once
#include <RetroMediaLab/MediaEngine/RuntimeDisk.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
RETROMEDIALAB_MEDIAENGINE_API void validateTrack(const MediaFileSystems::SectorImage&,std::int32_t,std::int32_t,std::span<const MediaDiskSector>);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> replaceTrack(const MediaFileSystems::SectorImage&,std::int32_t,std::int32_t,std::span<const MediaDiskSector>,bool retainGeometry,std::optional<std::span<const MediaFileSystems::SectorBlock>> previousTrack={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> projectTrack(const MediaFileSystems::SectorImage&,std::int32_t,std::int32_t,std::span<const MediaDiskSector>);
}
