#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
#include <map>
namespace RetroMediaLab::MediaEngine::DiskCopy {
using Bytes=MediaFileSystems::Bytes;
using SectorImage=MediaFileSystems::SectorImage;
struct Image {
    std::shared_ptr<const SectorImage> image;
    std::vector<std::uint8_t> name;
    std::uint8_t diskFormat=0,formatByte=0;
    std::optional<bool> dataChecksumValid,tagChecksumValid;
};
using Metadata=std::map<std::string,std::string>;
RETROMEDIALAB_MEDIAENGINE_API bool hasPrivateWord(Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t checksum(Bytes);
RETROMEDIALAB_MEDIAENGINE_API Image readDetailed(Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const SectorImage> read(Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool hasSupportedDataLayout(const SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const SectorImage&,const Image&);
RETROMEDIALAB_MEDIAENGINE_API Image createMetadata(std::shared_ptr<const SectorImage>,std::string_view name);
RETROMEDIALAB_MEDIAENGINE_API void write(const SectorImage&,const std::filesystem::path&,const Image&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(std::shared_ptr<const SectorImage>,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Metadata metadata(const Image&);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Image> restore(std::shared_ptr<const SectorImage>,const Metadata&);
}
