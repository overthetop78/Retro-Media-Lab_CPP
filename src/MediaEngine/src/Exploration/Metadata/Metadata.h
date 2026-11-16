#pragma once
#include <RetroMediaLab/MediaEngine/DiskMetadata.h>
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
namespace RetroMediaLab::MediaEngine::DiskMetadata {
struct AtnDetection {bool recognized=false;std::int32_t memberCount=0;};
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> systemId(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> protectionId(std::span<const std::string>);
RETROMEDIALAB_MEDIAENGINE_API AtnDetection detectAtnArchive(MediaFileSystems::Bytes,std::uint32_t sectorSize);
RETROMEDIALAB_MEDIAENGINE_API DiskContentMetadata content(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API DiskImageMetadata create(const MediaFileSystems::SectorImage&,std::span<const std::string> detectedFormats={},std::stop_token={});
}
