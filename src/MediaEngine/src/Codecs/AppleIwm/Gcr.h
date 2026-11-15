#pragma once
#include "Codec.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
namespace RetroMediaLab::MediaEngine::AppleIwm {
enum class Variant {Macintosh,LisaFileWare};
inline std::string_view codecId(Variant variant)noexcept{return variant==Variant::Macintosh?Constants::MacId:Constants::LisaId;}
inline std::string_view codecName(Variant variant)noexcept{return variant==Variant::Macintosh?Constants::MacName:Constants::LisaName;}
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,Variant=Variant::Macintosh,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decodeBits(Flux::BitView,Variant=Variant::Macintosh,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decodeAtBitCell(const Flux::Revolution&,double,Variant=Variant::Macintosh,std::stop_token = {});
class Encoder final:public Encoding::TrackEncoder {
 Variant variant_;
public:
 explicit Encoder(Variant variant=Variant::Macintosh)noexcept:variant_(variant){}
 std::string_view id()const noexcept override{return codecId(variant_);}
 std::string_view displayName()const noexcept override{return codecName(variant_);}
protected:
 RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&,std::stop_token)const override;
};
class Decoder final:public Decoding::Decoder {
 Variant variant_;
public:
 explicit Decoder(Variant variant=Variant::Macintosh)noexcept:variant_(variant){}
 bool participatesInAutomaticDetection()const noexcept override{return variant_!=Variant::LisaFileWare;}
 std::string_view id()const noexcept override{return codecId(variant_);}
 std::string_view displayName()const noexcept override{return codecName(variant_);}
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return AppleIwm::decode(revolution,variant_);}
};
}
