#include "Fm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::DataGeneral {
namespace {void validate(std::string_view field,std::int32_t value,std::int32_t maximum){if(value<0||value>maximum)throw MediaError(DiagnosticCode::DataGeneralAddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});}}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validate(Encoding::Constants::CylinderField,request.cylinder,MaximumCylinder);validate(Encoding::Constants::HeadField,request.head,MaximumHead);Flux::Bits bits;const auto sync=Encoding::compactFm(SyncBytes);
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::DataGeneralSectorSize,{std::to_string(sector.data.size())});validate(Encoding::Constants::NumberField,sector.number,MaximumSectorNumber);
 const std::array identity{std::uint8_t(request.cylinder|(request.head<<HeadShift)),std::uint8_t(sector.number<<SectorShift)};Encoding::raw(bits,sync);Encoding::fm(bits,identity);Encoding::gap(bits,HeaderGapBitCount);Encoding::raw(bits,sync);auto data=sector.data;const auto sum=checksum(data,token);data.push_back(std::uint8_t(sum>>BitsPerByte));data.push_back(std::uint8_t(sum));Encoding::fm(bits,data);Encoding::gap(bits,DataGapBitCount);
 }
 return bits;
}
}
