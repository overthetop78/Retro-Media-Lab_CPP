#pragma once
#include "../../Decoding/Decoder.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Models.h"
#include "Commodore1541Constants.h"
namespace RetroMediaLab::MediaEngine::Commodore1541Gcr {
class Encoder final:public Encoding::TrackEncoder {
public:
    std::string_view id()const noexcept override{return Constants::Id;}
    std::string_view displayName()const noexcept override{return Constants::DisplayName;}
protected:
    RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&,std::stop_token)const override;
};
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::int32_t diskTrack(std::int32_t cylinder,std::int32_t head) noexcept;
class Decoder final:public Decoding::Decoder {
public:
    std::string_view id()const noexcept override{return Constants::Id;}
    std::string_view displayName()const noexcept override{return Constants::DisplayName;}
    Decoding::Result decode(const Flux::Revolution& revolution)const override{return Commodore1541Gcr::decode(revolution);}
};
}
