#include "Commodore900.h"
#include "Gcr.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Commodore900Gcr {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;Flux::Bits bits;
 for(const auto& sector:request.sectors){
  checkCancellation(token);if(sector.data.size()!=SectorBytes)throw MediaError(DiagnosticCode::C900GcrSectorSize,{std::to_string(SectorBytes),std::to_string(sector.data.size())});
  if(sector.number<0||sector.number>MaximumAddress)throw MediaError(DiagnosticCode::TrackSectorNumber,{std::to_string(sector.number),std::to_string(MaximumAddress)});
  const std::vector<std::uint8_t> header{HeaderMark,std::uint8_t(request.cylinder),std::uint8_t(sector.number)};auto fullHeader=header;fullHeader.push_back(CommodoreGcr::checksum(header));
  std::vector<std::uint8_t> data;data.reserve(DataBytes);data.push_back(DataMark);data.insert(data.end(),sector.data.begin(),sector.data.end());data.push_back(CommodoreGcr::checksum(data));
  for(const auto* record:{&fullHeader,&data}){Encoding::gap(bits,SyncGap,true);const auto encoded=CommodoreGcr::encode(*record,token);if(encoded.size()>MaximumBits-bits.size())throw MediaError(DiagnosticCode::FluxSizeLimit);bits.insert(bits.end(),encoded.begin(),encoded.end());Encoding::gap(bits,RecordGap);}
 }
 return bits;
}
}
