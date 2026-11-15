#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
namespace RetroMediaLab::MediaEngine::Tycom {
struct Header { std::int64_t offset; std::uint8_t cylinder, sector; bool crcValid; std::vector<std::uint8_t> bytes; };
struct DataMark { std::int64_t offset; std::uint8_t mark; };
struct Data { DataMark mark; std::vector<std::uint8_t> payload, bytes; bool crcValid; };
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> decodeHeader(Flux::BitView, std::int64_t offset, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<DataMark> findNextDataMark(Flux::BitView, std::int64_t start, std::int64_t maximumDistance, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> decodeData(Flux::BitView, DataMark, std::stop_token = {});
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
    Decoding::Result decode(const Flux::Revolution& revolution) const override { return Tycom::decode(revolution); }
};
}

