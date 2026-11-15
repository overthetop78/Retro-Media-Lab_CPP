#include "Fm.h"
#include "../RotatingChecksum.h"
#include "../../Primitives/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Heathkit {
std::vector<std::uint8_t> encodeRecord(MediaFileSystems::Bytes values,std::stop_token token){
 checkCancellation(token);std::vector<std::uint8_t> result;result.reserve(values.size()+Constants::ChecksumByteCount);const auto checksum=RotatingChecksum::compute(values,token);for(auto value:values){checkCancellation(token);result.push_back(Primitives::reverseBits(value));}result.push_back(Primitives::reverseBits(checksum));return result;
}
Record decodeRecord(MediaFileSystems::Bytes encoded,std::stop_token token){
 checkCancellation(token);if(encoded.empty())throw MediaError(DiagnosticCode::HeathkitRecordEmpty);std::vector<std::uint8_t> payload;payload.reserve(encoded.size()-Constants::ChecksumByteCount);for(std::size_t index=0;index+Constants::ChecksumByteCount<encoded.size();++index){checkCancellation(token);payload.push_back(Primitives::reverseBits(encoded[index]));}const auto valid=RotatingChecksum::compute(payload,token)==Primitives::reverseBits(encoded.back());return {std::move(payload),valid};
}
}
