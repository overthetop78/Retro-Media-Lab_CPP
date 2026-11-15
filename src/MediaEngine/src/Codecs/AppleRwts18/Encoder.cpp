#include "Gcr.h"
#include "../AppleII/Constants.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleRwts18 {
namespace {
void validate(std::string_view field,std::int32_t value,std::int32_t maximum){if(value<0||value>maximum)throw MediaError(DiagnosticCode::Rwts18AddressRange,{std::string(field),std::to_string(value),std::to_string(maximum)});}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;validate(Encoding::Constants::CylinderField,request.cylinder,MaximumCylinder);
 std::array<const Encoding::Sector*,SectorCount> sectors{};
 bool valid=request.sectors.size()==SectorCount;
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.number<0||sector.number>=SectorCount||sectors[sector.number]){valid=false;continue;}sectors[sector.number]=&sector;}
 if(!valid)throw MediaError(DiagnosticCode::Rwts18TrackLayout,{std::to_string(request.sectors.size())});
 for(const auto& sector:request.sectors)if(sector.data.size()!=SectorByteCount)throw MediaError(DiagnosticCode::Rwts18SectorSize,{std::to_string(sector.number),std::to_string(sector.data.size())});
 const auto identifier=attribute(request.attributes,std::string(IdentifierAttribute),DefaultIdentifier);validate(IdentifierAttribute,identifier,MaximumIdentifier);
 Flux::Bits bits;const auto& table=AppleII::Constants::SixAndTwoTable;
 for(std::int32_t number=LastSectorNumber;number>=0;--number){
  checkCancellation(token);Encoding::gap(bits,number==LastSectorNumber?FirstSectorGapBitCount:OtherSectorGapBitCount,true);Encoding::raw(bits,AddressMarkBytes);
  const std::array<std::uint8_t,AddressByteCount> address{table[request.cylinder],table[number],table[request.cylinder^number],AddressTrailer};Encoding::raw(bits,address);
  const std::array<std::uint8_t,SeparatorByteCount> separator{SyncByte,SyncByte,std::uint8_t(identifier)};Encoding::raw(bits,separator);Encoding::raw(bits,encodePayload(sectors[number]->data,token));Encoding::raw(bits,DataTrailerBytes);
 }
 return bits;
}
}
