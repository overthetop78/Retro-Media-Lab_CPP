#include "Mmfm.h"
#include "../../Encoding/Bits.h"
#include "../../Primitives/Bits.h"
#include "../Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::HpMmfm {
namespace {void validateAddress(std::string_view field,std::int32_t value,std::int32_t maximum){if(value<0||value>maximum)throw MediaError(DiagnosticCode::HpAddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});}}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validateAddress(Encoding::Constants::CylinderField,request.cylinder,MaximumCylinder);validateAddress(Encoding::Constants::HeadField,request.head,MaximumHead);Flux::Bits bits;
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::HpSectorSize,{std::to_string(sector.data.size())});validateAddress(Encoding::Constants::NumberField,sector.number,MaximumSector);
 const std::array identity{Primitives::reverseBits(std::uint8_t(request.cylinder)),Primitives::reverseBits(std::uint8_t(sector.number|(request.head<<HeadShift)))};Encoding::raw(bits,SectorSync);Encoding::mfm(bits,Crc16::append(identity,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token));Encoding::gap(bits,HeaderGapBitCount);
 const auto encoded=encodePayload(sector.data,token);Encoding::raw(bits,DataSync);Encoding::mfm(bits,Crc16::append(encoded,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token));Encoding::gap(bits,DataGapBitCount);
 }
 return bits;
}
}
