#pragma once
#include "Flux.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::Flux {
RETROMEDIALAB_MEDIAENGINE_API bool match(BitView,std::int64_t offset,std::uint32_t pattern,std::int32_t length=Constants::UshortPatternBits) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool matchBytes(BitView,std::int64_t offset,std::span<const std::uint8_t>) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> mfmByte(BitView,std::int64_t offset) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> rawByte(BitView,std::int64_t offset) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> fmByte32(BitView,std::int64_t offset) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> mfmBytes(BitView,std::int64_t offset,std::int64_t count,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> fmBytes32(BitView,std::int64_t offset,std::int64_t count,std::stop_token={});
}
