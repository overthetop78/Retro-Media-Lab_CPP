#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"

namespace RetroMediaLab::MediaEngine::Aed6200p {
RETROMEDIALAB_MEDIAENGINE_API std::array<std::uint8_t, Constants::HeaderIdentityByteCount>
buildHeader(std::int32_t cylinder, std::int32_t sector, std::int32_t size, std::stop_token = {});
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
        return Aed6200p::decode(revolution);
    }
};
}

