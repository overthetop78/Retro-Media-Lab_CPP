#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::CpcDsk {
enum class ContainerKind {Standard,Extended};
struct Sector {std::uint8_t cylinder,head,id,sizeCode,status1,status2;std::vector<std::uint8_t> data;};
struct Track {std::int32_t index;bool present;std::uint8_t cylinder,head,sectorSizeCode,gap3Length,fillerByte;std::vector<Sector> sectors;};
struct Image {ContainerKind kind;std::uint8_t cylinders,heads;std::vector<Track> tracks;std::shared_ptr<const MediaFileSystems::SectorImage> sectorImage;};
RETROMEDIALAB_MEDIAENGINE_API bool canRead(MediaFileSystems::Bytes header,std::uint64_t length,std::optional<std::string_view> requestedFormat = {});
RETROMEDIALAB_MEDIAENGINE_API Image readDetailed(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Image readDetailed(const std::filesystem::path&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const std::filesystem::path&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Image buildImage(std::shared_ptr<const MediaFileSystems::SectorImage>,ContainerKind,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const Image&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const std::filesystem::path&,std::stop_token = {});
}
