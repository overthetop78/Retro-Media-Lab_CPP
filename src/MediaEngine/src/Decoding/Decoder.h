#pragma once
#include "Models.h"
#include "../Models/Flux/Flux.h"
#include <functional>
#include <memory>
namespace RetroMediaLab::MediaEngine::Decoding {
using SharedResult=std::shared_ptr<const Result>;
using DecodeOperation=std::function<Result(std::string_view,const Flux::SharedRevolution&)>;
class Decoder {
public:
    virtual ~Decoder()=default;
    virtual std::string_view id() const noexcept=0;
    virtual std::string_view displayName() const noexcept=0;
    virtual bool participatesInAutomaticDetection() const noexcept{return true;}
    virtual Result decode(const Flux::Revolution&) const=0;
};
}
