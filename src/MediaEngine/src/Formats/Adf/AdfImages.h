#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Adf {
struct Profile {MediaFileSystems::SectorGeometry geometry;std::int64_t firstSectorNumber=0;};
struct PartialDescription {Profile profile;std::uint64_t capacity;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<Profile> acornGeometries();
RETROMEDIALAB_MEDIAENGINE_API std::vector<Profile> amigaGeometries();
RETROMEDIALAB_MEDIAENGINE_API std::optional<Profile> findGeometry(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API bool hasDosPlusHeader(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::optional<PartialDescription> describePartial(MediaFileSystems::Bytes,std::uint64_t storedLength);
RETROMEDIALAB_MEDIAENGINE_API bool canRead(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::string_view extension,std::optional<std::string_view> requestedFormat = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::optional<std::string_view> requestedFormat = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::optional<std::string_view> requestedFormat = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume exploreAcorn(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::optional<std::string_view> requestedFormat = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaFileSystems::SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token = {});
}
