#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::TwoImg {
enum class ImageFormat:std::uint32_t {Dos,ProDos,Nib};
struct Header {std::uint16_t headerLength;ImageFormat imageFormat;std::uint32_t dataOffset,dataLength;};
RETROMEDIALAB_MEDIAENGINE_API Header readHeader(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> payload(const MediaFileSystems::SectorImage&,std::string_view target);
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view target,std::stop_token={});
}
