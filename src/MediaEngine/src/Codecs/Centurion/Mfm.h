#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Centurion {
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> buildHeader(std::uint8_t cylinder,std::uint8_t sector,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> buildDataField(MediaFileSystems::Bytes,std::stop_token = {});
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
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return Centurion::decode(revolution);}
};
}
