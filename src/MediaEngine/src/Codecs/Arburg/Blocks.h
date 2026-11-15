#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>

namespace RetroMediaLab::MediaEngine::Arburg {
RETROMEDIALAB_MEDIAENGINE_API std::uint16_t checksum(MediaFileSystems::Bytes, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool checksumValid(MediaFileSystems::Bytes, std::int64_t usefulSize, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> createBlock(MediaFileSystems::Bytes, std::int64_t totalSize, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> buildBlock(MediaFileSystems::Bytes, bool system, std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeSystem(MediaFileSystems::Bytes, std::stop_token = {});
struct SystemData {
    std::vector<std::uint8_t> bytes;
    std::int64_t endOffset;
};
RETROMEDIALAB_MEDIAENGINE_API std::optional<SystemData> decodeSystem(Flux::BitView, std::int64_t start, std::int64_t count, std::stop_token = {});
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
        return Arburg::decode(revolution);
    }
};
}

