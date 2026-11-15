#include "Mfm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Centurion {
namespace {void validate(std::string_view field,std::int32_t value){if(value<0||value>Constants::MaximumAddress)throw MediaError(DiagnosticCode::CenturionAddressRange,{std::string(field),std::to_string(value),std::to_string(Constants::MaximumAddress)});}}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validate(Encoding::Constants::CylinderField,request.cylinder);Flux::Bits bits;
 for(const auto& sector:request.sectors){checkCancellation(token);validate(Encoding::Constants::NumberField,sector.number);Encoding::raw(bits,SectorMark);Encoding::mfm(bits,buildHeader(std::uint8_t(request.cylinder),std::uint8_t(sector.number),token));Encoding::gap(bits,HeaderGapBitCount);Encoding::raw(bits,DataMark);Encoding::mfm(bits,buildDataField(sector.data,token));Encoding::gap(bits,DataGapBitCount);}
 return bits;
}
}
