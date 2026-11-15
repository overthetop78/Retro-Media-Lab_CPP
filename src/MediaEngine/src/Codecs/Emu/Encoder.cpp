#include "Fm.h"
#include "../../Encoding/Bits.h"
#include "../Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::EmuFm {
namespace {void validate(std::string_view field,std::int32_t value,std::int32_t maximum){if(value<0||value>maximum)throw MediaError(DiagnosticCode::EmuAddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});}}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validate(Encoding::Constants::CylinderField,request.cylinder,MaximumCylinder);validate(Encoding::Constants::HeadField,request.head,MaximumHead);const auto track=reverseBits(std::uint8_t((request.cylinder<<TrackShift)|request.head));Flux::Bits bits;
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::EmuSectorSize,{std::to_string(sector.data.size())});
 const std::array<std::uint8_t,1> identity{track};const auto header=Crc16::append(identity,Crc16::Constants::Ibm,Crc16::Constants::Zero,token);Encoding::raw(bits,SectorMark);Encoding::doubleFm(bits,header);Encoding::gap(bits,GapBitCount,true);
 const auto data=Crc16::append(sector.data,Crc16::Constants::Ibm,Crc16::Constants::Zero,token);Encoding::raw(bits,SectorMark);Encoding::doubleFm(bits,data);Encoding::gap(bits,GapBitCount,true);}
 return bits;
}
}
