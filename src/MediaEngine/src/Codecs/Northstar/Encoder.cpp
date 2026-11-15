#include "Mfm.h"
#include "../RotatingChecksum.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Northstar {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;Flux::Bits bits;const auto mark=Encoding::compactMfm(SyncBytes);
 for(const auto& sector:request.sectors){
  checkCancellation(token);
  if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::NorthstarSectorSize,{std::to_string(sector.data.size())});
  if(request.cylinder<0||request.cylinder>MaximumCylinder)throw MediaError(DiagnosticCode::NorthstarCylinderRange,{std::to_string(request.cylinder),std::to_string(MaximumCylinder)});
  if(sector.number<0||sector.number>MaximumSector)throw MediaError(DiagnosticCode::NorthstarSectorRange,{std::to_string(sector.number),std::to_string(MaximumSector)});
  Encoding::raw(bits,mark);const std::array address{pack(request.cylinder,sector.number)};Encoding::mfm(bits,address);Encoding::mfm(bits,sector.data);const std::array checksum{RotatingChecksum::compute(sector.data,token)};Encoding::mfm(bits,checksum);Encoding::gap(bits,GapBitCount);
 }
 return bits;
}
}
