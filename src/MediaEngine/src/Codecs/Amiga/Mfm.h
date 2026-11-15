#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <utility>
namespace RetroMediaLab::MediaEngine::AmigaMfm {
using Parity=std::pair<std::uint8_t,std::uint8_t>;
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> decodeOddEven(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodeOddEven(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Parity parity(MediaFileSystems::Bytes,std::int64_t offset,std::int64_t count,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Parity splitParity(MediaFileSystems::Bytes,std::int64_t offset,std::int64_t count,std::stop_token = {});
inline std::uint8_t packTrack(std::int32_t cylinder,std::int32_t head) noexcept{return std::uint8_t((std::uint32_t(cylinder)<<Constants::TrackCylinderShift)|(std::uint32_t(head)&Constants::TrackHeadMask));}
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
    Decoding::Result decode(const Flux::Revolution& revolution)const override{return AmigaMfm::decode(revolution);}
};
}
