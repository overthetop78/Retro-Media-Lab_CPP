#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Msa {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> pack(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> unpack(MediaFileSystems::Bytes,std::int32_t expected,std::int32_t cylinder=0,std::int32_t head=0);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
}
