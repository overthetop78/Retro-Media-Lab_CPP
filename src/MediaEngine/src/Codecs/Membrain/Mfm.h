#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"

namespace RetroMediaLab::MediaEngine::Membrain {
struct Address { std::uint8_t cylinder, head, sector; };
struct Header {
    std::int64_t offset;
    Address address;
    bool crcValid;
    std::vector<std::uint8_t> bytes;
};
struct Data {
    std::int64_t offset;
    std::uint8_t mark;
    std::vector<std::uint8_t> payload, bytes;
    bool crcValid;
};
RETROMEDIALAB_MEDIAENGINE_API std::array<std::uint8_t, Constants::AddressByteCount> pack(std::int32_t cylinder, std::int32_t head, std::int32_t sector) noexcept;
RETROMEDIALAB_MEDIAENGINE_API Address unpack(std::uint8_t cylinderHigh, std::uint8_t packed) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> decodeHeader(Flux::BitView, std::int64_t offset, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::int64_t findDataMark(Flux::BitView, std::int64_t start, std::int64_t end, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> decodeData(Flux::BitView, std::int64_t offset, std::stop_token = {});
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
    Decoding::Result decode(const Flux::Revolution& revolution) const override {
        return Membrain::decode(revolution);
    }
};
}

