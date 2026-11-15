#pragma once
#include "Constants.h"
#include "../../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <array>
#include <utility>
namespace RetroMediaLab::MediaEngine::AppleII {
struct DecodedBlock {std::vector<std::uint8_t> data;bool valid=false;std::int64_t endOffset=0;};
inline std::uint8_t decodeFourAndFour(std::uint8_t high,std::uint8_t low) noexcept{return std::uint8_t(((high<<1)|1)&low);}
RETROMEDIALAB_MEDIAENGINE_API std::pair<std::uint8_t,std::uint8_t> encodeFourAndFour(std::uint8_t) noexcept;
inline std::uint8_t addressChecksum(std::uint8_t volume,std::uint8_t cylinder,std::uint8_t sector) noexcept{return std::uint8_t(volume^cylinder^sector);}
RETROMEDIALAB_MEDIAENGINE_API std::array<std::uint8_t,Constants::EncodedAddressByteCount> encodeAddress(std::uint8_t volume,std::uint8_t cylinder,std::uint8_t sector) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodeSixAndTwo(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodeFiveAndThree(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<DecodedBlock> decodeSixAndTwo(Flux::BitView,std::int64_t offset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<DecodedBlock> decodeFiveAndThree(Flux::BitView,std::int64_t offset,std::stop_token = {});
}
