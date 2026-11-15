#pragma once
#include "../../Decoding/Decoder.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::RawFlux {
enum class AnomalyKind {None,LongInterval,ShortPulse};
RETROMEDIALAB_MEDIAENGINE_API std::int32_t convertToCellCount(std::uint32_t,double) noexcept;
RETROMEDIALAB_MEDIAENGINE_API AnomalyKind classify(std::uint32_t,double,std::int32_t) noexcept;
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
class Decoder final:public Decoding::Decoder {
public:
    std::string_view id()const noexcept override{return Constants::Id;}
    std::string_view displayName()const noexcept override{return Constants::SourceDisplayName;}
    std::string_view displayNameKey()const noexcept{return Constants::NameKey;}
    Decoding::Result decode(const Flux::Revolution& revolution)const override{return RawFlux::decode(revolution);}
};
}
