#include "Gcr.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Victor9000 {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;Flux::Bits bits;
 for(const auto& sector:request.sectors){
  checkCancellation(token);if(sector.data.size()!=SectorByteCount)throw MediaError(DiagnosticCode::VictorSectorSize,{std::to_string(sector.data.size())});
  if(sector.number<0||sector.number>MaximumSector)throw MediaError(DiagnosticCode::VictorSectorRange,{std::to_string(sector.number),std::to_string(MaximumSector)});
  const std::array<std::uint8_t,HeaderByteCount> header{HeaderType,std::uint8_t(request.cylinder),std::uint8_t(sector.number),headerChecksum(std::uint8_t(request.cylinder),std::uint8_t(sector.number)),HeaderId2,HeaderId1};addBlock(bits,HeaderMark,header,token);Encoding::gap(bits,HeaderGapBitCount);
  std::vector<std::uint8_t> data;data.reserve(DecodedDataByteCount);data.push_back(DataPrefix);data.insert(data.end(),sector.data.begin(),sector.data.end());const auto sum=checksum(sector.data,token);data.push_back(std::uint8_t(sum));data.push_back(std::uint8_t(sum>>BitsPerByte));addBlock(bits,DataMark,data,token);Encoding::gap(bits,DataGapBitCount);
 }
 return bits;
}
}
