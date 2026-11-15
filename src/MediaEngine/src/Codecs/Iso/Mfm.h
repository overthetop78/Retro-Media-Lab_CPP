#pragma once
#include "../../Decoding/Decoder.h"
#include "../../Encoding/Track.h"
#include "MfmConstants.h"
#include "../../Decoding/Models.h"
namespace RetroMediaLab::MediaEngine::IsoMfm {
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decodeCore(const Flux::Bitstream&,std::stop_token = {});
class Encoder final:public Encoding::TrackEncoder {
public:
    std::string_view id()const noexcept override{return Constants::Id;}
    std::string_view displayName()const noexcept override{return Constants::DisplayName;}
protected:
    RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&,std::stop_token)const override;
    RETROMEDIALAB_MEDIAENGINE_API Flux::Revolution encodeRevolution(const Encoding::Request&,Flux::BitView,std::stop_token)const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canEncodeEmptyTrack(const Encoding::Request&)const override;
};
class Decoder final:public Decoding::Decoder {
public:
    std::string_view id()const noexcept override{return Constants::Id;}
    std::string_view displayName()const noexcept override{return Constants::DisplayName;}
    Decoding::Result decode(const Flux::Revolution& revolution)const override{return IsoMfm::decode(revolution);}
};
}
