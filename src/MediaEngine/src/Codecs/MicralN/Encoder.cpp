#include "Fm.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::MicralN {
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;Flux::Bits bits;
 for(const auto& sector:request.sectors){checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::MicralSectorSize,{std::to_string(sector.data.size())});if(sector.number<0||sector.number>MaximumSectorNumber)throw MediaError(DiagnosticCode::MicralSectorRange,{std::to_string(sector.number),std::to_string(MaximumSectorNumber)});
 std::vector<std::uint8_t> block;block.reserve(BytesAfterMark);block.push_back(std::uint8_t(sector.number));block.push_back(std::uint8_t(request.cylinder));block.insert(block.end(),sector.data.begin(),sector.data.end());block.push_back(checksum(sector.data,token));Encoding::raw(bits,SectorMark);Encoding::fm(bits,block);Encoding::gap(bits,GapBitCount);}
 return bits;
}
}
