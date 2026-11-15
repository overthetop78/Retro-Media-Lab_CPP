#pragma once
#include "../../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Woz {
enum class Version {Woz1,Woz2};
struct Track {std::int32_t track;std::uint8_t descriptor;Flux::Bits bits;};
struct Tracks {Version version;std::vector<Track> tracks;};
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t crc32(MediaFileSystems::Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API Tracks readTracks(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void appendChunk(std::vector<std::uint8_t>&,std::string_view,MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(std::span<const Flux::Bits>);
RETROMEDIALAB_MEDIAENGINE_API void write(std::span<const Flux::Bits>,const std::filesystem::path&,std::stop_token={});
}
