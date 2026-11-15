#include "Mfm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Micropolis {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;Flux::Bits bits;const std::vector<std::uint8_t> preamble(PreambleByteCount);
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::MicropolisSectorSize,{std::to_string(sector.data.size())});if(sector.number<0||sector.number>MaximumSectorNumber)throw MediaError(DiagnosticCode::MicropolisSectorRange,{std::to_string(sector.number),std::to_string(MaximumSectorNumber)});const auto record=createRecord(std::uint8_t(request.cylinder),std::uint8_t(sector.number),sector.data,token);Encoding::mfm(bits,preamble);Encoding::mfm(bits,record.bytes);Encoding::gap(bits,GapBitCount);}
 return bits;
}
}
