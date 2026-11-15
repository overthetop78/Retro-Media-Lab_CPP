#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Commodore900 {
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorGeometry geometry();
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t sectorsPerTrack(std::int64_t cylinder);
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorAddress addressOf(std::int64_t logicalBlock);
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t logicalBlockOf(MediaFileSystems::SectorAddress);
RETROMEDIALAB_MEDIAENGINE_API bool canRead(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::optional<std::string_view> requested = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaFileSystems::SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token = {});
namespace Internal {bool sameFormat(std::string_view);}
}
