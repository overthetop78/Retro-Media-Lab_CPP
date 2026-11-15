#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::MicralN {
struct Block {std::uint8_t sector,cylinder;std::vector<std::uint8_t> data;std::uint8_t storedChecksum;bool valid;};
inline std::uint8_t updateChecksum(std::uint8_t checksum,std::uint8_t data)noexcept{const auto carrySource=((data^checksum)^Constants::ComplementMask)&((data+checksum)^data);return std::uint8_t(checksum+data+((carrySource&Constants::CarryMask)?1:0));}
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t checksum(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Block> decodeBlock(Flux::BitView,std::int64_t offset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
class Encoder final:public Encoding::TrackEncoder {
public:
 std::string_view id()const noexcept override{return Constants::Id;}
 std::string_view displayName()const noexcept override{return Constants::DisplayName;}
protected:
 RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&,std::stop_token)const override;
};
class Decoder final:public Decoding::Decoder {
public:
 std::string_view id()const noexcept override{return Constants::Id;}
 std::string_view displayName()const noexcept override{return Constants::DisplayName;}
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return MicralN::decode(revolution);}
};
}
