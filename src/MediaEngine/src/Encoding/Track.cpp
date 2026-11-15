#include "Track.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Encoding {
EncodedTrack TrackEncoder::encode(const Request& request,std::stop_token token)const {
 checkCancellation(token);if(request.cylinder<0||request.cylinder>Constants::MaximumCylinder)throw MediaError(DiagnosticCode::TrackCylinder,{std::to_string(request.cylinder),std::to_string(Constants::MaximumCylinder)});
 if(request.head<0||request.head>Constants::MaximumHead)throw MediaError(DiagnosticCode::TrackHead,{std::to_string(request.head),std::to_string(Constants::MaximumHead)});
 if(request.sectors.size()<Constants::MinimumSectors&&!canEncodeEmptyTrack(request))throw MediaError(DiagnosticCode::TrackMissingSectors,{std::to_string(request.sectors.size())});
 if(!request.cellTicks)throw MediaError(DiagnosticCode::FluxZeroCell);if(!request.indexTicks)throw MediaError(DiagnosticCode::FluxZeroIndex);
 auto bits=encodeBits(request,token);if(bits.empty())throw MediaError(DiagnosticCode::TrackEmpty,{std::string(id())});auto revolution=encodeRevolution(request,bits,token);return {std::string(id()),std::move(bits),std::move(revolution)};
}
Flux::Revolution TrackEncoder::encodeRevolution(const Request& request,Flux::BitView bits,std::stop_token token)const{return Flux::createRevolution(bits,request.cellTicks,request.indexTicks,true,token);}
std::int32_t TrackEncoder::attribute(const Attributes& values,const std::string& key,std::int32_t fallback){if(values){const auto found=values->find(key);if(found!=values->end())return found->second;}return fallback;}
Flux::Bits TrackEncoder::encodeIsoBits(const Request& request,const AddressValidator& validate,const std::function<std::uint8_t(const Sector&)>& size,const std::function<void(Flux::Bits&,const Request&,const Sector&,std::uint8_t)>& address,const std::function<void(Flux::Bits&,const Sector&)>& data,std::stop_token token){
 Flux::Bits bits;validate(Constants::CylinderField,request.cylinder);validate(Constants::HeadField,request.head);for(const auto& sector:request.sectors){checkCancellation(token);validate(Constants::NumberField,sector.number);const auto code=size(sector);address(bits,request,sector,code);data(bits,sector);}return bits;
}
}
