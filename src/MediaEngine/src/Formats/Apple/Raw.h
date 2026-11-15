#pragma once
#include "Probe.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Apple {
enum class MatchKind {ValidatedStructure,ExtensionHint,GeometryFallback};
struct ReadResult {std::shared_ptr<const MediaFileSystems::SectorImage> image;MatchKind match;};
RETROMEDIALAB_MEDIAENGINE_API ReadResult readRaw(Bytes,std::string_view extension,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool isProDos140(std::string_view) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> rawPayload(const MediaFileSystems::SectorImage&,std::string_view target,std::string_view extension);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> twoImgPayload(const MediaFileSystems::SectorImage&,std::string_view target);
RETROMEDIALAB_MEDIAENGINE_API bool hasMacintoshGeometry(const MediaFileSystems::SectorImage&) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> macintoshPayload(const MediaFileSystems::SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API void writeRaw(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::string_view target,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void writeMacintosh(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
}
