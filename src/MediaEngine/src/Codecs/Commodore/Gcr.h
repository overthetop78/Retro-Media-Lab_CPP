#pragma once
#include "../../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::CommodoreGcr {
struct DecodeResult {std::optional<std::vector<std::uint8_t>> bytes;std::uint64_t endOffset;};
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> decodeNibble(Flux::BitView,std::int64_t offset,std::int64_t stride = 1);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> decodeByte(Flux::BitView,std::int64_t offset,std::int64_t stride = 1);
RETROMEDIALAB_MEDIAENGINE_API DecodeResult decodeBytes(Flux::BitView,std::int64_t offset,std::int64_t count,std::int64_t stride = 1,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encode(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API void write(Flux::Bits& target,std::int64_t offset,MediaFileSystems::Bytes,std::int64_t stride,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t checksum(MediaFileSystems::Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool validChecksum(MediaFileSystems::Bytes) noexcept;
}
