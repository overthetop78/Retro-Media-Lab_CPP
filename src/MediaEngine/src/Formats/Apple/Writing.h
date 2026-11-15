#pragma once
#include "TrackEncoding.h"
#include <filesystem>
#include <functional>
namespace RetroMediaLab::MediaEngine::Apple {
RETROMEDIALAB_MEDIAENGINE_API bool sectorTarget(std::string_view format,std::string_view extension);
RETROMEDIALAB_MEDIAENGINE_API bool canWriteSector(const MediaFileSystems::SectorImage&,std::string_view target);
RETROMEDIALAB_MEDIAENGINE_API void validateSector(const MediaFileSystems::SectorImage&,std::string_view target);
RETROMEDIALAB_MEDIAENGINE_API bool nibbleTarget(std::string_view format,std::string_view extension);
class DiskImageWriter final {
public:
    using ByteWriter=std::function<void(const std::filesystem::path&,MediaFileSystems::Bytes,std::stop_token)>;
    DiskImageWriter()=default;
    explicit DiskImageWriter(TrackEncodingService encoder,ByteWriter writer={}):encoder_(std::move(encoder)),writer_(std::move(writer)){}
    RETROMEDIALAB_MEDIAENGINE_API static bool supportsExtension(std::string_view);
    RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={})const;
private:TrackEncodingService encoder_;ByteWriter writer_;
};
}
