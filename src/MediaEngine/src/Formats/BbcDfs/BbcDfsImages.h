#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/BbcDfs.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::BbcDfs {
using Geometry=MediaFileSystems::SectorGeometry;
RETROMEDIALAB_MEDIAENGINE_API std::vector<Geometry> geometries();
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> findGeometry(std::string_view);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> findGeometry(std::uint32_t heads,std::uint64_t capacity);
RETROMEDIALAB_MEDIAENGINE_API Geometry observedGeometry(std::int64_t cylinders,std::int64_t heads);
RETROMEDIALAB_MEDIAENGINE_API bool hasCatalogues(MediaFileSystems::Bytes,const Geometry&);
RETROMEDIALAB_MEDIAENGINE_API bool canRead(MediaFileSystems::Bytes header,std::uint64_t length,std::string_view extension,std::optional<std::string_view> requestedFormat = {});
RETROMEDIALAB_MEDIAENGINE_API Geometry detect(MediaFileSystems::Bytes header,std::uint64_t length,std::string_view extension);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::string_view extension,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::string_view extension,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaFileSystems::SectorImage&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view,std::stop_token = {});
}
