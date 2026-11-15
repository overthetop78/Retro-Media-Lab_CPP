#include "Fm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Heathkit {
namespace {void validate(std::string_view field,std::int32_t value){if(value<0||value>Constants::MaximumAddress)throw MediaError(DiagnosticCode::HeathkitAddressRange,{std::string(field),std::to_string(value),std::to_string(Constants::MaximumAddress)});}}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;const auto volume=attribute(request.attributes,std::string(VolumeAttributeName),DefaultVolume);validate(VolumeAttributeName,volume);validate(Encoding::Constants::CylinderField,request.cylinder);Flux::Bits bits;const auto mark=Encoding::compactFm(SyncBytes);
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::HeathkitSectorSize,{std::to_string(sector.data.size())});validate(Encoding::Constants::NumberField,sector.number);
 const std::array header{std::uint8_t(volume),std::uint8_t(request.cylinder),std::uint8_t(sector.number)};Encoding::raw(bits,mark);Encoding::fm(bits,encodeRecord(header,token));Encoding::gap(bits,HeaderGapBitCount);Encoding::raw(bits,mark);Encoding::fm(bits,encodeRecord(sector.data,token));Encoding::gap(bits,DataGapBitCount);
 }
 return bits;
}
}
