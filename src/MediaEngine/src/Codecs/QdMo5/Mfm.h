#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::QdMo5 {
struct Header { std::int64_t offset; std::int32_t sector; std::vector<std::uint8_t> reserved; };
struct Data { std::int64_t offset; std::uint8_t prefix; std::vector<std::uint8_t> payload; std::uint8_t storedChecksum; bool checksumValid; };
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t checksum(std::uint8_t prefix, MediaFileSystems::Bytes, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> decodeHeader(Flux::BitView, std::int64_t offset, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> decodeData(Flux::BitView, std::int64_t offset, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::int64_t findNextData(Flux::BitView, std::int64_t start, std::int64_t maximumDistance, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&, std::stop_token = {});
class Encoder final : public Encoding::TrackEncoder {
public:
    std::string_view id() const noexcept override { return Constants::Id; }
    std::string_view displayName() const noexcept override { return Constants::DisplayName; }
protected:
    RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&, std::stop_token) const override;
};
class Decoder final : public Decoding::Decoder {
public:
    std::string_view id() const noexcept override { return Constants::Id; }
    std::string_view displayName() const noexcept override { return Constants::DisplayName; }
    Decoding::Result decode(const Flux::Revolution& revolution) const override { return QdMo5::decode(revolution); }
};
}

