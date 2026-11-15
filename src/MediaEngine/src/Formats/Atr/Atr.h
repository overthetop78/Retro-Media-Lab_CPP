#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Atr {
RETROMEDIALAB_MEDIAENGINE_API bool supportedSectorSize(std::int32_t) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t bootAreaLength(std::int32_t) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::int32_t sectorCount(std::int32_t payloadLength,std::int32_t sectorSize);
RETROMEDIALAB_MEDIAENGINE_API bool truncatedSingleDensity(std::int32_t sectorSize,std::int32_t count) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::size_t usablePayloadLength(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API void validate(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaFileSystems::SectorImage&,std::string_view target);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage&,std::string_view target,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view target,std::stop_token={});
}
