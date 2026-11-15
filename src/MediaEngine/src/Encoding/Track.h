#pragma once
#include "../Models/Flux/Flux.h"
#include "Constants.h"
#include <string>
#include <map>
#include <functional>
namespace RetroMediaLab::MediaEngine::Encoding {
using Attributes=std::optional<std::map<std::string,std::int32_t>>;
struct Sector {std::int32_t number=0;std::vector<std::uint8_t> data;bool deleted=false;std::optional<std::uint8_t> sizeCode;Attributes attributes;std::optional<std::int32_t> recordedCylinder,recordedHead;std::optional<std::uint8_t> recordedSizeCode;std::optional<bool> headerIntegrityValid,dataIntegrityValid;std::vector<std::uint8_t> recordedErrorCheckCode;};
struct Request {std::int32_t cylinder=0,head=0;std::vector<Sector> sectors;Attributes attributes;std::uint32_t cellTicks=Constants::DefaultCellTicks,indexTicks=Constants::DefaultIndexTicks;};
struct EncodedTrack {std::string encoderId;Flux::Bits bits;Flux::Revolution revolution;};
struct DiskTrack {std::int32_t cylinder=0,head=0;std::uint32_t cellTicks=Constants::DefaultCellTicks;EncodedTrack track;};
class TrackEncoder {
public:
 virtual ~TrackEncoder()=default;virtual std::string_view id()const noexcept=0;virtual std::string_view displayName()const noexcept=0;
 RETROMEDIALAB_MEDIAENGINE_API EncodedTrack encode(const Request&,std::stop_token = {})const;
protected:
 virtual Flux::Bits encodeBits(const Request&,std::stop_token)const=0;
 RETROMEDIALAB_MEDIAENGINE_API virtual Flux::Revolution encodeRevolution(const Request&,Flux::BitView,std::stop_token)const;
 virtual bool canEncodeEmptyTrack(const Request&)const{return false;}
 RETROMEDIALAB_MEDIAENGINE_API static std::int32_t attribute(const Attributes&,const std::string&,std::int32_t fallback);
 using AddressValidator=std::function<void(std::string_view,std::int32_t)>;
 RETROMEDIALAB_MEDIAENGINE_API static Flux::Bits encodeIsoBits(const Request&,const AddressValidator&,const std::function<std::uint8_t(const Sector&)>&,const std::function<void(Flux::Bits&,const Request&,const Sector&,std::uint8_t)>&,const std::function<void(Flux::Bits&,const Sector&)>&,std::stop_token = {});
};
}
