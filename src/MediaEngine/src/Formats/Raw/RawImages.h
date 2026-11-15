#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Raw {
using Geometry=MediaFileSystems::SectorGeometry;
RETROMEDIALAB_MEDIAENGINE_API std::vector<Geometry> ibmGeometries();
RETROMEDIALAB_MEDIAENGINE_API std::vector<Geometry> msxGeometries();
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> ibmFromCapacity(std::uint64_t);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> ibmFromDescriptor(std::uint8_t);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> ibmFromBoot(MediaFileSystems::Bytes,std::uint8_t);
RETROMEDIALAB_MEDIAENGINE_API Geometry detectIbm(MediaFileSystems::Bytes boot,std::uint64_t imageLength);
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeMsx(MediaFileSystems::Bytes boot,std::uint64_t imageLength);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Geometry> msxFromCapacity(std::uint64_t,std::uint8_t);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readIbm(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readMsx(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume exploreIbm(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume exploreMsx(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool canWriteIbm(const MediaFileSystems::SectorImage&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API bool canWriteMsx(const MediaFileSystems::SectorImage&,std::string_view);
RETROMEDIALAB_MEDIAENGINE_API void writeIbm(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API void writeMsx(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view,std::stop_token = {});
}
